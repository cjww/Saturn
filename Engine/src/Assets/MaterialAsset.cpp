#include "pch.h"
#include "MaterialAsset.h"

#include "AssetManager.h"

namespace sa {
	bool MaterialAsset::create(const std::string& name) {
		m_name = name;

		m_header.type = type();
		m_isLoaded = true;
		return true;
	}

	bool MaterialAsset::load() {
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_isLoaded)
			return false;

		SA_DEBUG_LOG_INFO("Loading MaterialAsset ", getID());
		
		std::ifstream file(m_assetPath, std::ios::binary);
		if (!file.good()) {
			file.close();
			return false;
		}
		m_header = readHeader(file);

		file.read((char*)&data.values, sizeof(data.values));

		uint32_t typeCount = 0;
		file.read((char*)&typeCount, sizeof(typeCount));
		
		for (uint32_t i = 0; i < typeCount; i++) {
			sa::MaterialTextureType type;
			file.read((char*)&type, sizeof(type));
			uint32_t textureCount = 0;
			file.read((char*)&textureCount, sizeof(textureCount));
			data.m_textures[type].resize(textureCount);

			for (uint32_t j = 0; j < textureCount; j++) {
				UUID textureID;
				file.read((char*)&textureID, sizeof(textureID));
				TextureAsset* pTextureAsset = AssetManager::get().getAsset<TextureAsset>(textureID);
				if (pTextureAsset) {
					m_taskExecutor.async([pTextureAsset]() {
						pTextureAsset->load();
					});
				}
				data.m_textures[type][j] = textureID;
			}

		}

		for (uint32_t i = 0; i < typeCount; i++) {
			sa::MaterialTextureType type;
			file.read((char*)&type, sizeof(type));
			uint32_t blendCount = 0;
			file.read((char*)&blendCount, sizeof(blendCount));
			data.m_blending[type].resize(blendCount);
			file.read((char*)data.m_blending[type].data(), sizeof(std::pair<sa::TextureBlendOp, float>) * blendCount);
		}

		file.close();
		m_isLoaded = true;

		data.m_allTextures.clear();
		return true;
	}

	bool MaterialAsset::write() {
		if (!m_isLoaded)
			return false;

		std::ofstream file(m_assetPath, std::ios::binary | std::ios::out);
		if (!file.good()) {
			file.close();
			return false;
		}
		
		writeHeader(m_header, file);
		
		// Values
		file.write((char*)&data.values, sizeof(data.values));

		//Textures
		uint32_t typeCount = data.m_textures.size();
		file.write((char*)&typeCount, sizeof(typeCount));

		for (auto& [type, assets] : data.m_textures) {
			file.write((char*)&type, sizeof(type));
			uint32_t textureCount = assets.size();
			file.write((char*)&textureCount, sizeof(textureCount));
			for (auto& textureID : assets) {
				TextureAsset* pTextureAsset = AssetManager::get().getAsset<TextureAsset>(textureID);
				if (pTextureAsset)
					pTextureAsset->write();
				file.write((char*)&textureID, sizeof(UUID));
			}
		}

		for (auto& [type, blendings] : data.m_blending) {
			file.write((char*)&type, sizeof(type));
			uint32_t blendingCount = blendings.size();
			file.write((char*)&blendingCount, sizeof(blendingCount));
			file.write((char*)blendings.data(), sizeof(std::pair<sa::TextureBlendOp, float>));
		}

		file.close();
		return true;
	}
}
