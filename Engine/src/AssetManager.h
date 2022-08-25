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
		sa::Buffer vertexBuffer;
		sa::Buffer indexBuffer;
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
	

		AssetManager();

		void loadAssimpModel(const std::filesystem::path& path, ModelData* pModel, ProgressView<ResourceID> progress);

	public:
		~AssetManager();
	
		static AssetManager& get();

		Texture2D* loadDefaultTexture();
		Texture2D* loadTexture(const std::filesystem::path& path, bool generateMipMaps);

		ResourceID loadModel(const std::filesystem::path& path, ProgressView<ResourceID> progress = {});
		ProgressView<ResourceID> loadModelAsync(const std::filesystem::path& path);

		ResourceID loadQuad();

		ModelData* getModel(ResourceID id) const;
		Material* getMaterial(ResourceID id) const;

	};

}