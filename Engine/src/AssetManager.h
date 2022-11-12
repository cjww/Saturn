#pragma once

#include <glm/vec4.hpp>
#include <glm/vec2.hpp>

#include <filesystem>

#include <Vertex.h>

#include "Renderer.hpp"

#include "structs.h"

#include "Graphics/Material.h"

#include "taskflow\taskflow.hpp"

#include "ProgressView.h"

#include "ECS\Entity.h"
#include "ECS/Components.h"
//--------------------------------------------------------------------------------------
// AssetManager is the class that will hold all assets. 
//--------------------------------------------------------------------------------------


namespace sa {

	struct Mesh {
		std::vector<VertexNormalUV> vertices;
		std::vector<uint32_t> indices;

		ResourceID materialID;
	};

	struct ModelData {
		std::vector<Mesh> meshes;
	};

	// Singelton class
	class AssetManager
	{
	private:	
		tf::Executor m_taskExecutor;

		ResourceID m_nextID;
	
		std::mutex m_mutex;

		std::unordered_map<std::string, ProgressView<ResourceID>> m_loadingModels;

		std::unordered_map<ResourceID, ModelData*> m_models;
		std::unordered_map<ResourceID, Texture2D*> m_textures;
		std::unordered_map<ResourceID, Material*> m_materials;


		AssetManager();

		void loadAssimpModel(const std::filesystem::path& path, ModelData* pModel, ProgressView<ResourceID>& progress);
		ResourceID loadModel(const std::filesystem::path& path, ProgressView<ResourceID>& progress);
	public:
		~AssetManager();
	
		static AssetManager& get();

		void clear();

		Texture2D* loadDefaultTexture();
		Texture2D* loadDefaultBlackTexture();

		Texture2D* loadTexture(const std::filesystem::path& path, bool generateMipMaps);

		std::tuple<ResourceID, ModelData*> newModel(const std::string& name = "");

		ProgressView<ResourceID>& loadModel(const std::filesystem::path& path);

		ResourceID loadDefaultMaterial();

		ResourceID loadQuad();
		ResourceID loadBox();

		ModelData* getModel(ResourceID id) const;
		Material* getMaterial(ResourceID id) const;

	};

}