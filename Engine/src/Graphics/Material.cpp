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
			targetBlend[i] = { blendedTex.blendOp, blendedTex.blendFactor};
		}
	}

	Material::Material()
		: twoSided(false)
	{
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
		m_allTextures.clear();
	}

	void Material::setTextures(const std::vector<BlendedTexture>& textures, MaterialTextureType type) {
		setTextures(textures, type, values.diffuseMapCount);
	}

	const std::vector<Texture>& Material::fetchTextures() {
		if (!m_allTextures.empty())
			return m_allTextures;

		for (auto& [type, textures] : m_textures) {
			for (auto& id : textures) {
				TextureAsset* asset = AssetManager::get().getAsset<TextureAsset>(id);
				if (asset && asset->isLoaded() && asset->getTexture().isValid()) {
					m_allTextures.push_back(asset->getTexture());
				}
				else {
					SA_DEBUG_LOG_WARNING("Texture asset invalid (UUID: ", asset->getHeader().id, "). Loaded default texture");
					m_allTextures.push_back(*AssetManager::get().loadDefaultTexture());
				}
			}
		}
		return m_allTextures;
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