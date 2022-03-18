#pragma once

#include <glm/vec4.hpp>
#include <glm/vec2.hpp>

#include <filesystem>

#include <Vertex.h>

#include "Graphics\Buffer.h"

//--------------------------------------------------------------------------------------
// ResourceManager is the class that will hold all resources. 
//--------------------------------------------------------------------------------------

typedef uint32_t ResourceID;
#define NULL_RESOURCE (ResourceID)-1

namespace vr {
	struct Buffer;
}

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
	class ResourceManager
	{
	private:	

		ResourceID m_nextID;
	
		std::unordered_map<std::string, ResourceID> m_resourceIDs;
		std::unordered_map<ResourceID, std::unique_ptr<ModelData>> m_models;
		

		ResourceManager();

	public:
		~ResourceManager();
	
		static ResourceManager& get();

		ResourceID loadModel(const std::filesystem::path& path);
		ResourceID loadQuad();

		ModelData* getModel(ResourceID id) const;

		// creates empty model
		std::tuple<ResourceID, ModelData*> createModel();


		std::string fetchResourceName(ResourceID resource) const;
	};

}