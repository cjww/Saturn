#pragma once
#include <unordered_map>
#include <memory>
#include <filesystem>

#include <glm/vec4.hpp>
#include <glm/vec2.hpp>

#include <Renderer.hpp>
#include <Vertex.h>

//--------------------------------------------------------------------------------------
// ResourceManager is the class that will hold all resources. 
//--------------------------------------------------------------------------------------

typedef uint32_t ResourceID;
#define NULL_RESOURCE (ResourceID)-1

namespace sa {

	struct Material {
		ResourceID diffuseMap = NULL_RESOURCE;
		ResourceID normalMap = NULL_RESOURCE;
		ResourceID specularMap = NULL_RESOURCE;

		glm::vec4 diffuseColor = glm::vec4(1, 1, 1, 1);
		glm::vec4 specularColor = glm::vec4(1, 1, 1, 1);

	};

	struct Mesh {
		vr::Buffer* vertexBuffer;
		vr::Buffer* indexBuffer;
		union {
			size_t indexCount, vertexCount;
		};
		Material material;

	};

	struct ModelData {
		std::vector<Mesh> meshes;

	};

	// Singelton class
	class ResourceManager
	{
	private:	
		static ResourceManager* m_myInstance;

		ResourceID m_nextID;
	
		std::unordered_map<std::string, ResourceID> m_resourceIDs;
		std::unordered_map<ResourceID, std::unique_ptr<ModelData>> m_models;
	

		ResourceManager();

	public:
		~ResourceManager();
	
		static ResourceManager* get();
		static void cleanup();
	
		ResourceID loadModel(const std::filesystem::path& path);
		ResourceID loadQuad();

		ModelData* getModel(ResourceID id) const;

	};

}