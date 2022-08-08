#pragma once
#include "structs.hpp"
#include "Resources/ResourceManager.hpp"
#include "Resources/Texture.hpp"
#include "Resources/Buffer.hpp"
#include "RenderContext.hpp"

#include "Tools/Logger.hpp"

#define MAX_TEXTURE_MAP_COUNT 8

namespace sa {

	class Material {
	private:
		ResourceID m_pipeline;
		ResourceID m_sampler;
		std::vector<Texture> m_diffuseMaps;
		std::vector<Texture> m_normalMaps;
		std::vector<Texture> m_specularMaps;
		Buffer m_valueBuffer;
		ResourceID m_descriptorSet;

		void setMaps(const std::vector<Texture2D>& textures, std::vector<Texture>& targetMaps, uint32_t& count);

	public:
		struct Values { // sent to shader
			Color diffuseColor = { 1, 1, 1, 1 };
			Color specularColor = { 1, 1, 1, 1 };

			uint32_t diffuseMapCount = 0;
			uint32_t normalMapCount = 0;
			uint32_t specularMapCount = 0;
			
			float roughness;

		} values;

		Material();

		void init(ResourceID pipeline, ResourceID sampler);

		bool isInitialized() const;

		void update();
		void bind(RenderContext& context, ResourceID pipeline, ResourceID sampler);
	
		void setDiffuseMaps(const std::vector<Texture2D>& diffuseTextures);
		void setNormalMaps(const std::vector<Texture2D>& normalTextures);
		void setSpecularMaps(const std::vector<Texture2D>& specularTextures);

		bool operator==(const Material& other);

	};
}