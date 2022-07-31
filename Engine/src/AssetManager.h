#pragma once

#include <glm/vec4.hpp>
#include <glm/vec2.hpp>

#include <filesystem>

#include <Vertex.h>

#include "Renderer.hpp"

#include "structs.h"

//--------------------------------------------------------------------------------------
// AssetManager is the class that will hold all assets. 
//--------------------------------------------------------------------------------------


namespace sa {

	struct Material {
		ResourceID pipeline;
		ResourceID sampler;
		sa::Texture2D diffuseMap;
		sa::Texture2D normalMap;
		sa::Texture2D specularMap;

		struct Values {
			Color diffuseColor = { 1, 1, 1, 1 };
			Color specularColor = { 1, 1, 1, 1 };

			float roughness;
		} values;
		sa::Buffer valueBuffer;

		ResourceID descriptorSet;
	};

	struct Mesh {
		sa::Buffer vertexBuffer;
		sa::Buffer indexBuffer;
	};

	struct ModelData {
		std::vector<Mesh> meshes;
	};

	// Singelton class
	class AssetManager
	{
	private:	

		ResourceID m_nextID;
	

		AssetManager();

	public:
		~AssetManager();
	
		static AssetManager& get();

		ResourceID newMaterial(ResourceID pipeline);

		ResourceID loadModel(const std::filesystem::path& path);
		ResourceID loadQuad();

		ModelData* getModel(ResourceID id) const;
		Material* getMaterial(ResourceID id) const;

	};

}