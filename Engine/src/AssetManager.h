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

#include "Tools/Profiler.h"

#include "Assets\IAsset.h"

#define SA_ASSET_DIR "Assets/"

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

	
	class AssetPackage {
	public:
		std::filesystem::path path;
		size_t size;
		std::fstream file;

		~AssetPackage() {
			file.close();
		}
	};

	// Singelton class
	class AssetManager
	{
	private:	
		tf::Executor m_taskExecutor;

		std::mutex m_mutex;

		std::unordered_map<std::string, ProgressView<ResourceID>> m_loadingModels;

		std::unordered_map<ResourceID, ModelData*> m_models;
		std::unordered_map<ResourceID, Texture2D*> m_textures;
		std::unordered_map<ResourceID, Material*> m_materials;


		// Assets
		std::unordered_map<UUID, std::unique_ptr<IAsset>> m_assets;
		std::list<AssetPackage> m_assetPackages;

		AssetManager();

		void loadAssimpModel(const std::filesystem::path& path, ModelData* pModel, ProgressView<ResourceID>& progress);
		ResourceID loadModel(const std::filesystem::path& path, ProgressView<ResourceID>& progress);

		void locateAssetPackages();
		void locateStandaloneAssets();
		IAsset* addAsset(const std::filesystem::path& assetPath);

		void loadAssetPackage(AssetPackage& package);

		AssetPackage& newAssetPackage(const std::filesystem::path& path);
		
		template<typename T, typename ...Args>
		T* allocateAsset(Args&&... args);


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
		
		const std::unordered_map<UUID, std::unique_ptr<IAsset>>& getAssets() const;

		void rescanAssets();

		template<typename T>
		T* getAsset(UUID id) const;

		template<typename T>
		T* importAsset(const std::filesystem::path& path);

		void removeAsset(IAsset* asset);
		void removeAsset(UUID id);


	};

	template<typename T>
	inline T* AssetManager::getAsset(UUID id) const {
		return dynamic_cast<T*>(m_assets.at(id).get());
	}

	template<typename T>
	inline T* AssetManager::importAsset(const std::filesystem::path& path) {
		UUID id;
		auto [it, success] = m_assets.insert({ id, std::make_unique<T>(id) });
		IAsset* asset = it->second.get();

		auto filename = path.filename().replace_extension(".asset");
		asset->setAssetPath(SA_ASSET_DIR / filename); // The path the asset will write to

		if (!asset->importFromFile(path)) {
			removeAsset(asset);
			return nullptr;
		}
		return static_cast<T*>(asset);
	}
}