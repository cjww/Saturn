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
		m_pMaterialShader = nullptr;
	}

	void Material::update() {
		values.albedoMapCount = m_textures[MaterialTextureType::BASE_COLOR].size();
		values.albedoMapFirst = 0;
		
		values.normalMapCount = m_textures[MaterialTextureType::NORMALS].size();
		values.normalMapFirst = values.albedoMapCount;
		
		values.metalnessMapCount = m_textures[MaterialTextureType::METALNESS].size();
		values.metalnessMapFirst = values.normalMapFirst + values.normalMapCount;
		
		values.emissiveMapCount = m_textures[MaterialTextureType::EMISSIVE].size();
		values.emissiveMapFirst = values.metalnessMapFirst + values.metalnessMapCount;

		values.occlusionMapCount = m_textures[MaterialTextureType::LIGHTMAP].size();
		values.occlusionMapFirst = values.emissiveMapFirst + values.emissiveMapCount;
		m_allTexturesLoaded = false;
	}

	void Material::setTextures(const std::vector<BlendedTexture>& textures, MaterialTextureType type) {
		setTextures(textures, type, values.albedoMapCount);
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

	std::unordered_map<MaterialTextureType, std::vector<UUID>>& Material::getTextures() {
		return m_textures;
	}

	MaterialShader* Material::getMaterialShader() const {
		return m_pMaterialShader;
	}

	void Material::setMaterialShader(MaterialShader* pMaterialShader) {
		m_pMaterialShader = pMaterialShader;
	}

	bool Material::onLoad(std::ifstream& file, AssetLoadFlags flags) {
		m_textures.clear();
		m_blending.clear();
		m_allTextures.clear();

		file.read((char*)&values, sizeof(values));
		
		//MaterialShader
		UUID materialShaderID = 0;
		file.read((char*)&materialShaderID, sizeof(UUID));
		m_pMaterialShader = AssetManager::get().getAsset<MaterialShader>(materialShaderID);
		if (m_pMaterialShader)	{
			m_pMaterialShader->load(flags);
			addDependency(m_pMaterialShader->getProgress());
		}
		else {
			SA_DEBUG_LOG_WARNING("onLoad: No material shader attached to material ", getName());
		}

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
					pTextureAsset->load(flags);
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

		//MaterialShader
		UUID materialShaderID = 0;
		if (m_pMaterialShader) {
			materialShaderID = m_pMaterialShader->getID();
		}
		else {
			SA_DEBUG_LOG_WARNING("onWrite: No MaterialShader attached to material ", getName());
		}

		file.write((char*)&materialShaderID, sizeof(UUID));

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

		if(m_pMaterialShader)
			m_pMaterialShader->release();
		return true;
	}
	
	std::string Material::TextureTypeToString(MaterialTextureType type) {
		switch (type) {
		case MaterialTextureType::AMBIENT:
			return "Ambient";
		case MaterialTextureType::AMBIENT_OCCLUSION:
			return "Ambient Occlusion";
		case MaterialTextureType::BASE_COLOR:
			return "Base Color";
		case MaterialTextureType::CLEARCOAT:
			return "Clearcoat";
		case MaterialTextureType::DIFFUSE:
			return "Diffuse";
		case MaterialTextureType::DIFFUSE_ROUGHNESS:
			return "Diffuse Roughness";
		case MaterialTextureType::DISPLACEMENT:
			return "Displacement";
		case MaterialTextureType::EMISSION_COLOR:
			return "Emmision Color";
		case MaterialTextureType::EMISSIVE:
			return "Emissive";
		case MaterialTextureType::HEIGHT:
			return "Height";
		case MaterialTextureType::LIGHTMAP:
			return "Lightmap";
		case MaterialTextureType::METALNESS:
			return "Metalness";
		case MaterialTextureType::NONE:
			return "None";
		case MaterialTextureType::NORMALS:
			return "Normals";
		case MaterialTextureType::NORMAL_CAMERA:
			return "Normal Camera";
		case MaterialTextureType::OPACITY:
			return "Opacity";
		case MaterialTextureType::REFLECTION:
			return "Reflection";
		case MaterialTextureType::SHEEN:
			return "Sheen";
		case MaterialTextureType::SHININESS:
			return "Shininess";
		case MaterialTextureType::SPECULAR:
			return "Specular";
		case MaterialTextureType::TRANSMISSION:
			return "Transmission";
		case MaterialTextureType::UNKNOWN:
			return "Unknown";
		default:
			return "-";
		}
	}

}