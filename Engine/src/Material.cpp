#include "pch.h"
#include "Graphics/Material.h"

#include <vulkan/vulkan_core.h>

#include "Renderer.hpp"
#include "structs.h"

#include "AssetManager.h"

namespace sa {
	const char* to_string(MaterialTextureType type) {
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
			return "None";
		};
	}

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
		: Asset(header)
	{
		twoSided = false;
		m_allTexturesLoaded = false;
	}

	void Material::update() {
		values.albedoMapCount = m_textures[MaterialTextureType::BASE_COLOR].size();
		values.albedoMapFirst = 0;
		
		values.normalMapCount = m_textures[MaterialTextureType::NORMAL_CAMERA].size();
		values.normalMapFirst = values.albedoMapCount;
		
		values.metalnessMapCount = m_textures[MaterialTextureType::METALNESS].size();
		values.metalnessMapFirst = values.normalMapFirst + values.normalMapCount;

		values.roughnessMapCount = m_textures[MaterialTextureType::DIFFUSE_ROUGHNESS].size();
		values.roughnessMapFirst = values.metalnessMapFirst + values.metalnessMapCount;

		values.emissiveMapCount = m_textures[MaterialTextureType::EMISSION_COLOR].size();
		values.emissiveMapFirst = values.roughnessMapFirst + values.roughnessMapCount;

		values.occlusionMapCount = m_textures[MaterialTextureType::AMBIENT_OCCLUSION].size();
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
			for (auto& texture : textures) {
				TextureAsset* asset = texture.getAsset();
				if (asset && asset->getTexture().isValid()) {
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

	std::unordered_map<MaterialTextureType, std::vector<AssetHolder<TextureAsset>>>& Material::getTextures() {
		return m_textures;
	}

	const AssetHolder<MaterialShader>& Material::getMaterialShader() const {
		return m_materialShader;
	}

	void Material::setMaterialShader(UUID id) {
		m_materialShader = id;
	}

	bool Material::onLoad(std::ifstream& file, AssetLoadFlags flags) {
		m_textures.clear();
		m_blending.clear();
		m_allTextures.clear();

		file.read(reinterpret_cast<char*>(&values), sizeof(values));
		
		//MaterialShader
		UUID materialShaderID = SA_DEFAULT_MATERIAL_SHADER_ID;
		file.read(reinterpret_cast<char*>(&materialShaderID), sizeof(UUID));
		m_materialShader = materialShaderID;

		if(const auto pProgress = m_materialShader.getProgress())
			addDependency(*pProgress);

		uint32_t typeCount = 0;
		file.read(reinterpret_cast<char*>(&typeCount), sizeof(typeCount));

		for (uint32_t i = 0; i < typeCount; i++) {
			sa::MaterialTextureType type;
			file.read(reinterpret_cast<char*>(&type), sizeof(type));
			uint32_t textureCount = 0;
			file.read(reinterpret_cast<char*>(&textureCount), sizeof(textureCount));
			m_textures[type].resize(textureCount);

			for (uint32_t j = 0; j < textureCount; j++) {
				UUID textureID;
				file.read(reinterpret_cast<char*>(&textureID), sizeof(textureID));
				
				m_textures[type][j] = textureID;
				if(const auto pProgress = m_textures[type][j].getProgress())
					addDependency(*pProgress);
			}

		}

		for (uint32_t i = 0; i < typeCount; i++) {
			sa::MaterialTextureType type;
			file.read(reinterpret_cast<char*>(&type), sizeof(type));
			uint32_t blendCount = 0;
			file.read(reinterpret_cast<char*>(&blendCount), sizeof(blendCount));
			m_blending[type].resize(blendCount);
			file.read(reinterpret_cast<char*>(m_blending[type].data()), sizeof(std::pair<sa::TextureBlendOp, float>) * blendCount);
		}

		return true;
	}

	bool Material::onWrite(std::ofstream& file, AssetWriteFlags flags) {
		// Values
		file.write(reinterpret_cast<char*>(&values), sizeof(values));

		//MaterialShader
		UUID materialShaderID = m_materialShader ? m_materialShader.getID() : static_cast<UUID>(SA_DEFAULT_MATERIAL_SHADER_ID);
		file.write(reinterpret_cast<char*>(&materialShaderID), sizeof(UUID));

		//Textures
		uint32_t typeCount = m_textures.size();
		file.write(reinterpret_cast<char*>(&typeCount), sizeof(typeCount));

		for (auto& [type, assets] : m_textures) {
			file.write(reinterpret_cast<const char*>(&type), sizeof(type));
			uint32_t textureCount = assets.size();
			file.write(reinterpret_cast<char*>(&textureCount), sizeof(textureCount));
			for (auto& textureID : assets) {
				file.write(reinterpret_cast<const char*>(&textureID.getID()), sizeof(UUID));
			}
		}

		for (auto& [type, blendings] : m_blending) {
			file.write(reinterpret_cast<const char*>(&type), sizeof(type));
			uint32_t blendingCount = blendings.size();
			file.write(reinterpret_cast<const char*>(&blendingCount), sizeof(blendingCount));
			if (blendingCount > 0)
				file.write(reinterpret_cast<const char*>(blendings.data()), sizeof(std::pair<sa::TextureBlendOp, float>));
		}

		return true;
	}

	bool Material::onUnload() {
		
		m_allTextures.clear();
		m_allTextures.shrink_to_fit();
		m_allTexturesLoaded = false;

		std::unordered_map<MaterialTextureType, std::vector<std::pair<TextureBlendOp, float>>> tmpBlend;
		m_blending.swap(tmpBlend);

		std::unordered_map<MaterialTextureType, std::vector<AssetHolder<TextureAsset>>> tmpTex;
		m_textures.swap(tmpTex);

		m_materialShader = nullptr;

		return true;
	}
	
	

}