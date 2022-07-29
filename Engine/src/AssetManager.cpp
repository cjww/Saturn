#include "pch.h"
#include "AssetManager.h"

namespace sa {

	AssetManager& AssetManager::get() {
		static AssetManager instance;
		return instance;
	}

	ResourceID AssetManager::loadModel(const std::filesystem::path& path) {
		return ResourceManager::get().insert<ModelData>();
	}

	ResourceID AssetManager::loadQuad() {
		ResourceManager& resManager = ResourceManager::get();
		Renderer& renderer = Renderer::get();

		ResourceID id = resManager.keyToID<ModelData>("Quad");
		if (id != NULL_RESOURCE) {
			return id;
		}

		
		Mesh mesh = {};
		mesh.material = {};

		std::vector<VertexUV> vertices = {
			{ glm::vec4(-0.5f, 0.5f, 0, 1), glm::vec2(0, 0) },
			{ glm::vec4(0.5f, 0.5f, 0, 1), glm::vec2(1, 0) },
			{ glm::vec4(0.5f, -0.5f, 0, 1), glm::vec2(1, 1) },
			{ glm::vec4(-0.5f, -0.5f, 0, 1), glm::vec2(0, 1) }
		};


		mesh.vertexBuffer = renderer.createBuffer(BufferType::VERTEX, vertices.size() * sizeof(VertexUV), vertices.data());

		std::vector<uint32_t> indices = {
			0, 1, 3,
			1, 2, 3
		};
		mesh.indexBuffer = renderer.createBuffer(BufferType::INDEX, indices.size() * sizeof(uint32_t), indices.data());

		ModelData model = {};
		model.meshes.push_back(mesh);
		
		id = resManager.insert<ModelData>(model);

		return id;
	}

	ModelData* AssetManager::getModel(ResourceID id) const {
		return ResourceManager::get().get<ModelData>(id);
	}

	AssetManager::AssetManager()
		: m_nextID(0)
	{
	}

	AssetManager::~AssetManager() {

	}
}