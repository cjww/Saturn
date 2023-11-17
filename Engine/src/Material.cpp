#include "pch.h"
#include "Graphics/Material.h"
#include "Renderer.hpp"
#include "structs.h"

#include "AssetManager.h"

namespace sa {
	std::string to_string(MaterialTextureType type) {
		const static std::unordered_map<MaterialTextureType, std::string> map = {
			{ MaterialTextureType::AMBIENT, "Ambient" },
			{ MaterialTextureType::AMBIENT_OCCLUSION, "Ambient Occlusion" },
			{ MaterialTextureType::BASE_COLOR, "Base Color" },
			{ MaterialTextureType::CLEARCOAT, "Clearcoat" },
			{ MaterialTextureType::DIFFUSE, "Diffuse" },
			{ MaterialTextureType::DIFFUSE_ROUGHNESS, "Diffuse Roughness" },
			{ MaterialTextureType::DISPLACEMENT, "Displacement" },
			{ MaterialTextureType::EMISSION_COLOR, "Emmision Color" },
			{ MaterialTextureType::EMISSIVE, "Emissive" },
			{ MaterialTextureType::HEIGHT, "Height" },
			{ MaterialTextureType::LIGHTMAP, "Lightmap" },
			{ MaterialTextureType::METALNESS, "Metalness" },
			{ MaterialTextureType::NONE, "None" },
			{ MaterialTextureType::NORMALS, "Normals" },
			{ MaterialTextureType::NORMAL_CAMERA, "Normal Camera" },
			{ MaterialTextureType::OPACITY, "Opacity" },
			{ MaterialTextureType::REFLECTION, "Reflection" },
			{ MaterialTextureType::SHEEN, "Sheen" },
			{ MaterialTextureType::SHININESS, "Shininess" },
			{ MaterialTextureType::SPECULAR, "Specular" },
			{ MaterialTextureType::TRANSMISSION, "Transmission" },
			{ MaterialTextureType::UNKNOWN, "Unknown" },
		};

		return map.at(type);
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

		file.read((char*)&values, sizeof(values));
		
		//MaterialShader
		UUID materialShaderID = 0;
		file.read((char*)&materialShaderID, sizeof(UUID));
		m_materialShader = materialShaderID;

		auto pProgress = m_materialShader.getProgress();
		if(pProgress)
			addDependency(*pProgress);

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
				
				m_textures[type][j] = textureID;
				auto pProgress = m_textures[type][j].getProgress();
				if(pProgress)
					addDependency(*pProgress);
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
		UUID materialShaderID = m_materialShader.getID();

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