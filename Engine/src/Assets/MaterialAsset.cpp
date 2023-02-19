#include "pch.h"
#include "MaterialAsset.h"

#include "AssetManager.h"

namespace sa {
	bool MaterialAsset::create(const std::string& name) {
		m_name = name;
		m_isLoaded = true;
		return true;
	}

	bool MaterialAsset::load() {
		return dispatchLoad([&](std::ifstream& file) {
			data.m_textures.clear();
			data.m_blending.clear();
			data.m_allTextures.clear();

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
						pTextureAsset->load();
						m_progress.addDependency(&pTextureAsset->getProgress());
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

			return true;
		});
	}

	bool MaterialAsset::write() {
		return dispatchWrite([&](std::ofstream& file) {
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
					file.write((char*)&textureID, sizeof(UUID));
				}
			}

			for (auto& [type, blendings] : data.m_blending) {
				file.write((char*)&type, sizeof(type));
				uint32_t blendingCount = blendings.size();
				file.write((char*)&blendingCount, sizeof(blendingCount));
				if(blendingCount > 0)
					file.write((char*)blendings.data(), sizeof(std::pair<sa::TextureBlendOp, float>));
			}

			return true;
		});
	}

	bool MaterialAsset::unload() {
		for (auto& [type, textures] : data.m_textures) {
			for (auto& id : textures) {
				IAsset* pTexture = AssetManager::get().getAsset(id);
				if (pTexture)
					pTexture->release();
			}
		}
		data.m_allTextures.clear();
		data.m_allTextures.shrink_to_fit();
		data.m_allTexturesLoaded = false;

		std::unordered_map<MaterialTextureType, std::vector<std::pair<TextureBlendOp, float>>> tmpBlend;
		data.m_blending.swap(tmpBlend);

		std::unordered_map<MaterialTextureType, std::vector<UUID>> tmpTex;
		data.m_textures.swap(tmpTex);

		m_isLoaded = false;
		return true;
	}
}
