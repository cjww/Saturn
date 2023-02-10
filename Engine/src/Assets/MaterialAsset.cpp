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
		return false;
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
				file << textureID;
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
