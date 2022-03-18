#include "pch.h"
#include "ResourceManager.h"

#include <Graphics/Vulkan/Renderer.hpp>

namespace sa {

	ResourceManager& ResourceManager::get()
	{
		static ResourceManager instance;
		return instance;
	}

	ResourceID ResourceManager::loadModel(const std::filesystem::path& path)
	{
		if (m_resourceIDs.find(path.string()) != m_resourceIDs.end()) {
			return m_resourceIDs.at(path.string());
		}

		ResourceID id = m_nextID++;
		const auto& [reference, success] = m_models.insert(std::make_pair(id, std::make_unique<ModelData>()));
		ModelData* model = reference->second.get();


		return id;
	}

	ResourceID ResourceManager::loadQuad()
	{
		if (m_resourceIDs.find("Quad") != m_resourceIDs.end()) {
			return m_resourceIDs.at("Quad");
		}

		
		auto [id, model] = createModel();
		m_resourceIDs["Quad"] = id;
		
		Mesh mesh = {};
		mesh.material = {};

		std::vector<VertexUV> vertices = {
			{ glm::vec4(-0.5f, 0.5f, 0, 1), glm::vec2(0, 0) },
			{ glm::vec4(0.5f, 0.5f, 0, 1), glm::vec2(1, 0) },
			{ glm::vec4(0.5f, -0.5f, 0, 1), glm::vec2(1, 1) },
			{ glm::vec4(-0.5f, -0.5f, 0, 1), glm::vec2(0, 1) }
		};
		mesh.vertexBuffer = sa::Buffer(sa::BufferType::VERTEX, vertices.size() * sizeof(VertexUV), vertices.size(), vertices.data());

		std::vector<uint32_t> indices = {
			0, 1, 3,
			1, 2, 3
		};
		mesh.indexBuffer = sa::Buffer(sa::BufferType::INDEX, indices.size() * sizeof(uint32_t), indices.size(), indices.data());

		model->meshes.push_back(mesh);


		return id;
	}

	ModelData* ResourceManager::getModel(ResourceID id) const {
		return m_models.at(id).get();
	}

	std::tuple<ResourceID, ModelData*> ResourceManager::createModel() {
		ResourceID id = m_nextID++;
		const auto& [reference, success] = m_models.insert(std::make_pair(id, std::make_unique<ModelData>()));
		return std::make_tuple(id, reference->second.get());
	}

	std::string ResourceManager::fetchResourceName(ResourceID resource) const {
		for (auto [name, id] : m_resourceIDs){
			if (id == resource) {
				return name;
			}
		}
		return "-";
	}

	ResourceManager::ResourceManager()
		: m_nextID(0)
	{
	}

	ResourceManager::~ResourceManager() {

	}
}