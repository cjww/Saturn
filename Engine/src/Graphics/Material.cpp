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
			if (!blendedTex.texture.isValid()) {
				SA_DEBUG_LOG_WARNING("Texture invalid, loading default texture");
				targetMaps[i] = *AssetManager::get().loadDefaultTexture();
				targetBlend[i++] = { blendedTex.blendOp, blendedTex.blendFactor };
				continue;
			}
			targetMaps[i] = blendedTex.texture;
			targetBlend[i++] = { blendedTex.blendOp, blendedTex.blendFactor};
		}
	}

	Material::Material()
		: m_pipeline(NULL_RESOURCE)
		, m_sampler(NULL_RESOURCE)
		, m_descriptorSet(NULL_RESOURCE)
		, twoSided(false)
	{
	}

	void Material::init(ResourceID pipeline, ResourceID sampler) {
		m_pipeline = pipeline;
		m_sampler = sampler;
		if(!m_valueBuffer.isValid())
			m_valueBuffer = Renderer::get().createBuffer(BufferType::UNIFORM, sizeof(Values), &values);
		if (m_descriptorSet != NULL_RESOURCE) {
			Renderer::get().freeDescriptorSet(m_descriptorSet);
		}
		m_descriptorSet = Renderer::get().allocateDescriptorSet(pipeline, SET_MAT);

		update();
	}

	bool Material::isInitialized() const {
		return m_descriptorSet != NULL_RESOURCE && m_pipeline != NULL_RESOURCE && m_sampler != NULL_RESOURCE;
	}

	void Material::update() {
		Renderer& renderer = Renderer::get();
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

		if (m_valueBuffer.isValid()) {
			m_valueBuffer.write(values);
			renderer.updateDescriptorSet(m_descriptorSet, 0, m_valueBuffer);
		}

		if (m_sampler != NULL_RESOURCE)
			renderer.updateDescriptorSet(m_descriptorSet, 1, m_sampler);

		std::vector<Texture> textures = m_textures[MaterialTextureType::DIFFUSE];
		textures.insert(textures.end(), m_textures[MaterialTextureType::NORMALS].begin(), m_textures[MaterialTextureType::NORMALS].end());
		textures.insert(textures.end(), m_textures[MaterialTextureType::SPECULAR].begin(), m_textures[MaterialTextureType::SPECULAR].end());
		textures.insert(textures.end(), m_textures[MaterialTextureType::EMISSIVE].begin(), m_textures[MaterialTextureType::EMISSIVE].end());
		textures.insert(textures.end(), m_textures[MaterialTextureType::LIGHTMAP].begin(), m_textures[MaterialTextureType::LIGHTMAP].end());

		renderer.updateDescriptorSet(m_descriptorSet, 2, textures);

	}

	void Material::bind(RenderContext& context, ResourceID pipeline, ResourceID sampler) {
		if (!isInitialized() || pipeline != m_pipeline) {
			init(pipeline, sampler);
		}
		context.bindDescriptorSet(m_descriptorSet, pipeline);
	}

	void Material::setTextures(const std::vector<BlendedTexture>& textures, MaterialTextureType type) {
		setTextures(textures, type, values.diffuseMapCount);
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