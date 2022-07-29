#pragma once

#include <glm/vec4.hpp>
#include <glm/vec2.hpp>

#include <filesystem>

#include <Vertex.h>

#include "Renderer.hpp"

//--------------------------------------------------------------------------------------
// AssetManager is the class that will hold all assets. 
//--------------------------------------------------------------------------------------


namespace sa {

	struct Material {
		ResourceID diffuseMap = NULL_RESOURCE;
		ResourceID normalMap = NULL_RESOURCE;
		ResourceID specularMap = NULL_RESOURCE;

		glm::vec4 diffuseColor = glm::vec4(1, 1, 1, 1);
		glm::vec4 specularColor = glm::vec4(1, 1, 1, 1);

	};

	struct Mesh {
		sa::Buffer vertexBuffer;
		sa::Buffer indexBuffer;
		Material material;
	};

	struct ModelData {
		std::vector<Mesh> meshes;

	};

	// Singelton class
	class AssetManager
	{
	private:	

		ResourceID m_nextID;
	
		std::unordered_map<ResourceID, std::unique_ptr<ModelData>> m_models;
		

		AssetManager();

	public:
		~AssetManager();
	
		static AssetManager& get();

		ResourceID loadModel(const std::filesystem::path& path);
		ResourceID loadQuad();

		ModelData* getModel(ResourceID id) const;
		
	};

}