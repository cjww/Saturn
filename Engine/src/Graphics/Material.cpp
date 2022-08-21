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
				DEBUG_LOG_WARNING("Texture invalid, loading default texture");
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
		values.normalMapCount = m_textures[MaterialTextureType::NORMALS].size();
		values.specularMapCount = m_textures[MaterialTextureType::SPECULAR].size();

		if (m_valueBuffer.isValid()) {
			m_valueBuffer.write(values);
			renderer.updateDescriptorSet(m_descriptorSet, 0, m_valueBuffer);
		}

		if (m_sampler != NULL_RESOURCE)
			renderer.updateDescriptorSet(m_descriptorSet, 1, m_sampler);

		std::vector<Texture> textures = m_textures[MaterialTextureType::DIFFUSE];
		textures.insert(textures.end(), m_textures[MaterialTextureType::NORMALS].begin(), m_textures[MaterialTextureType::NORMALS].end());

		renderer.updateDescriptorSet(m_descriptorSet, 2, textures);

		/*
		renderer.updateDescriptorSet(m_descriptorSet, 3, m_textures[MaterialTextureType::NORMALS]);
		renderer.updateDescriptorSet(m_descriptorSet, 4, m_textures[MaterialTextureType::SPECULAR]);
		while (m_diffuseMaps.size() < MAX_TEXTURE_MAP_COUNT) {
			m_diffuseMaps.push_back(*sa::AssetManager::get().loadDefaultTexture());
			values.diffuseMapCount = m_diffuseMaps.size();
		}
		renderer.updateDescriptorSet(m_descriptorSet, 2, m_diffuseMaps);

		while (m_normalMaps.size() < MAX_TEXTURE_MAP_COUNT) {
			m_normalMaps.push_back(*sa::AssetManager::get().loadDefaultTexture());
			values.normalMapCount = m_normalMaps.size();
		}
		renderer.updateDescriptorSet(m_descriptorSet, 3, m_normalMaps);

		while (m_specularMaps.size() < MAX_TEXTURE_MAP_COUNT) {
			m_specularMaps.push_back(*sa::AssetManager::get().loadDefaultTexture());
			values.specularMapCount = m_specularMaps.size();
		}
		renderer.updateDescriptorSet(m_descriptorSet, 4, m_specularMaps);
		*/


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

	bool Material::operator==(const Material& other) {
		return values.diffuseColor == other.values.diffuseColor &&
			values.specularColor == other.values.specularColor &&
			utils::equals(values.roughness, other.values.roughness);
			
			//&&
			//m_diffuseMaps == other.m_diffuseMaps &&
			//m_normalMaps == other.m_normalMaps &&
			//m_specularMaps == other.m_specularMaps;
	}

}