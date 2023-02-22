#include "pch.h"
#include "Material.h"
#include "Renderer.hpp"
#include "structs.h"

#include "AssetManager.h"

namespace sa {
	void Material::setTextures(const std::vector<BlendedTexture>& textures, MaterialTextureType type, uint32_t& count) {
		count = std::min((uint32_t)textures.size(), MAX_TEXTURE_MAP_COUNT);
		if (count == 0)
			return;

		auto& targetMaps = m_textures[type];
		auto& targetBlend = m_blending[type];

		targetMaps.resize(count);
		targetBlend.resize(count);

		for (uint32_t i = 0; i < count; i++) {
			const BlendedTexture& blendedTex = textures[i];
			targetMaps[i] = blendedTex.textureAssetID;
			targetBlend[i] = { blendedTex.blendOp, blendedTex.blendFactor };
		}
	}

	Material::Material(const AssetHeader& header)
		: IAsset(header)
	{
		twoSided = false;
		m_allTexturesLoaded = false;
	}

	void Material::update() {
		values.diffuseMapCount = m_textures[MaterialTextureType::DIFFUSE].size();
		values.diffuseMapFirst = 0;
		
		values.normalMapCount = m_textures[MaterialTextureType::NORMALS].size();
		values.normalMapFirst = values.diffuseMapCount;
		
		values.specularMapCount = m_textures[MaterialTextureType::SPECULAR].size();
		values.specularMapFirst = values.normalMapFirst + values.normalMapCount;
		
		values.emissiveMapCount = m_textures[MaterialTextureType::EMISSIVE].size();
		values.emissiveMapFirst = values.specularMapFirst + values.specularMapCount;

		values.lightMapCount = m_textures[MaterialTextureType::LIGHTMAP].size();
		values.lightMapFirst = values.emissiveMapFirst + values.emissiveMapCount;
	}

	void Material::setTextures(const std::vector<BlendedTexture>& textures, MaterialTextureType type) {
		setTextures(textures, type, values.diffuseMapCount);
	}

	const std::vector<Texture>& Material::fetchTextures() {
		if (m_allTexturesLoaded)
			return m_allTextures;
		
		m_allTextures.clear();
		m_allTexturesLoaded = true;
		for (auto& [type, textures] : m_textures) {
			for (auto& id : textures) {
				TextureAsset* asset = AssetManager::get().getAsset<TextureAsset>(id);
				if (asset && asset->isLoaded() && asset->getTexture().isValid()) {
					m_allTextures.push_back(asset->getTexture());
				}
				else {
					m_allTextures.push_back(*AssetManager::get().loadDefaultTexture());
					m_allTexturesLoaded = false;
				}
			}
		}
		return m_allTextures;
	}


	bool Material::onLoad(std::ifstream& file, AssetLoadFlags flags) {
		m_textures.clear();
		m_blending.clear();
		m_allTextures.clear();

		file.read((char*)&values, sizeof(values));

		uint32_t typeCount = 0;
		file.read((char*)&typeCount, sizeof(typeCount));

		for (uint32_t i = 0; i < typeCount; i++) {
			sa::MaterialTextureType type;
			file.read((char*)&type, sizeof(type));
			uint32_t textureCount = 0;
			file.read((char*)&textureCount, sizeof(textureCount));
			m_textures[type].resize(textureCount);

			for (uint32_t j = 0; j < textureCount; j++) {
				UUID textureID;
				file.read((char*)&textureID, sizeof(textureID));
				TextureAsset* pTextureAsset = AssetManager::get().getAsset<TextureAsset>(textureID);
				if (pTextureAsset) {
					pTextureAsset->load();
					addDependency(pTextureAsset->getProgress());
				}
				m_textures[type][j] = textureID;
			}

		}

		for (uint32_t i = 0; i < typeCount; i++) {
			sa::MaterialTextureType type;
			file.read((char*)&type, sizeof(type));
			uint32_t blendCount = 0;
			file.read((char*)&blendCount, sizeof(blendCount));
			m_blending[type].resize(blendCount);
			file.read((char*)m_blending[type].data(), sizeof(std::pair<sa::TextureBlendOp, float>) * blendCount);
		}

		return true;
	}

	bool Material::onWrite(std::ofstream& file, AssetWriteFlags flags) {
		// Values
		file.write((char*)&values, sizeof(values));

		//Textures
		uint32_t typeCount = m_textures.size();
		file.write((char*)&typeCount, sizeof(typeCount));

		for (auto& [type, assets] : m_textures) {
			file.write((char*)&type, sizeof(type));
			uint32_t textureCount = assets.size();
			file.write((char*)&textureCount, sizeof(textureCount));
			for (auto& textureID : assets) {
				file.write((char*)&textureID, sizeof(UUID));
			}
		}

		for (auto& [type, blendings] : m_blending) {
			file.write((char*)&type, sizeof(type));
			uint32_t blendingCount = blendings.size();
			file.write((char*)&blendingCount, sizeof(blendingCount));
			if (blendingCount > 0)
				file.write((char*)blendings.data(), sizeof(std::pair<sa::TextureBlendOp, float>));
		}

		return true;
	}

	bool Material::onUnload() {
		for (auto& [type, textures] : m_textures) {
			for (auto& id : textures) {
				IAsset* pTexture = AssetManager::get().getAsset(id);
				if (pTexture)
					pTexture->release();
			}
		}
		m_allTextures.clear();
		m_allTextures.shrink_to_fit();
		m_allTexturesLoaded = false;

		std::unordered_map<MaterialTextureType, std::vector<std::pair<TextureBlendOp, float>>> tmpBlend;
		m_blending.swap(tmpBlend);

		std::unordered_map<MaterialTextureType, std::vector<UUID>> tmpTex;
		m_textures.swap(tmpTex);
		return true;
	}
	
	std::string toString(MaterialTextureType type) {
		switch (type) {
		case MaterialTextureType::AMBIENT:
			return "AMBIENT";
		case MaterialTextureType::AMBIENT_OCCLUSION:
			return "AMBIENT_OCCLUSION";
		case MaterialTextureType::BASE_COLOR:
			return "BASE_COLOR";
		case MaterialTextureType::CLEARCOAT:
			return "CLEARCOAT";
		case MaterialTextureType::DIFFUSE:
			return "DIFFUSE";
		case MaterialTextureType::DIFFUSE_ROUGHNESS:
			return "DIFFUSE_ROUGHNESS";
		case MaterialTextureType::DISPLACEMENT:
			return "DISPLACEMENT";
		case MaterialTextureType::EMISSION_COLOR:
			return "EMISSION_COLOR";
		case MaterialTextureType::EMISSIVE:
			return "EMISSIVE";
		case MaterialTextureType::HEIGHT:
			return "HEIGHT";
		case MaterialTextureType::LIGHTMAP:
			return "LIGHTMAP";
		case MaterialTextureType::METALNESS:
			return "METALNESS";
		case MaterialTextureType::NONE:
			return "NONE";
		case MaterialTextureType::NORMALS:
			return "NORMALS";
		case MaterialTextureType::NORMAL_CAMERA:
			return "NORMAL_CAMERA";
		case MaterialTextureType::OPACITY:
			return "OPACITY";
		case MaterialTextureType::REFLECTION:
			return "REFLECTION";
		case MaterialTextureType::SHEEN:
			return "SHEEN";
		case MaterialTextureType::SHININESS:
			return "SHININESS";
		case MaterialTextureType::SPECULAR:
			return "SPECULAR";
		case MaterialTextureType::TRANSMISSION:
			return "TRANSMISSION";
		case MaterialTextureType::UNKNOWN:
			return "UNKNOWN";
		default:
			return "-";
		}
	}

}