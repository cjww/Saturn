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

	class ModelAsset;
	class MaterialAsset;

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
	class AssetManager {
	private:	
		std::mutex m_mutex;

		std::unordered_map<ResourceID, Texture2D*> m_textures;

		std::unordered_map<std::filesystem::path, UUID> m_importedAssets;
		std::unordered_map<UUID, std::unique_ptr<IAsset>> m_assets;
		std::list<AssetPackage> m_assetPackages;

		UUID m_quad;
		UUID m_box;
		UUID m_defaultMaterial;

		AssetTypeID m_nextTypeID;
		std::unordered_map<AssetTypeID, std::function<IAsset* (const AssetHeader&)>> m_assetAddConversions;
		std::unordered_map<AssetTypeID, std::string> m_typeStrings;


		AssetManager();

		void locateAssetPackages();
		void locateStandaloneAssets();
		IAsset* addAsset(const std::filesystem::path& assetPath);

		void loadAssetPackage(AssetPackage& package);

		AssetPackage& newAssetPackage(const std::filesystem::path& path);

	public:
		~AssetManager();
	
		static AssetManager& get();

		void clear();

		Texture2D* loadDefaultTexture();
		Texture2D* loadDefaultBlackTexture();

		Texture2D* loadTexture(const std::filesystem::path& path, bool generateMipMaps);

		MaterialAsset* loadDefaultMaterial();

		ModelAsset* loadQuad();
		ModelAsset* loadBox();

		const std::unordered_map<UUID, std::unique_ptr<IAsset>>& getAssets() const;

		void rescanAssets();

		template<typename T>
		AssetTypeID registerAssetType();

		const std::string& getAssetTypeName(AssetTypeID typeID) const;

		template<typename T>
		T* getAsset(UUID id) const;
		IAsset* getAsset(UUID id) const;

		template<typename T>
		T* importAsset(const std::filesystem::path& path, const std::filesystem::path& assetDirectory = SA_ASSET_DIR);

		template<typename T>
		T* createAsset(const std::string& name, const std::filesystem::path& assetDirectory = SA_ASSET_DIR);

		void removeAsset(IAsset* asset);
		void removeAsset(UUID id);

	};

	template<typename T>
	inline AssetTypeID AssetManager::registerAssetType() {
		AssetTypeID id = m_nextTypeID++;
		
		m_assetAddConversions[id] = [&](const AssetHeader& header) {
			return m_assets.insert({ header.id, std::make_unique<T>(header) }).first->second.get();
		};
		T::s_typeID = id;

		std::string str = typeid(T).name();
		utils::stripTypeName(str);
		m_typeStrings[id] = str;

		return id;
	}

	template<typename T>
	inline T* AssetManager::getAsset(UUID id) const {
		if (!m_assets.count(id)) 
			return nullptr;
		return dynamic_cast<T*>(m_assets.at(id).get());
	}

	template<typename T>
	inline T* AssetManager::importAsset(const std::filesystem::path& path, const std::filesystem::path& assetDirectory) {
		AssetHeader header; // generates new UUID
		header.type = T::type();
		IAsset* asset;
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			if (m_importedAssets.count(path)) {
				return dynamic_cast<T*>(m_assets.at(m_importedAssets.at(path)).get());
			}
			m_importedAssets[path] = header.id;
			auto [it, success] = m_assets.insert({ header.id, std::make_unique<T>(header) });
			asset = it->second.get();
		}
		
		auto filename = path.filename().replace_extension(".asset");
		asset->setAssetPath(assetDirectory / filename); // The path the asset will write to

		if (!asset->importFromFile(path)) {
			std::lock_guard<std::mutex> lock(m_mutex);
			m_importedAssets.erase(path);
			removeAsset(asset);
			return nullptr;
		}
		
		return static_cast<T*>(asset);
	}
	
	template<typename T>
	inline T* AssetManager::createAsset(const std::string& name, const std::filesystem::path& assetDirectory) {
		AssetHeader header; // generates new UUID
		header.type = T::type();
		m_mutex.lock();
		auto [it, success] = m_assets.insert({ header.id, std::make_unique<T>(header) });
		m_mutex.unlock();
		IAsset* asset = it->second.get();

		if (!assetDirectory.empty()) {
			std::filesystem::path filename = name + ".asset";
			asset->setAssetPath(assetDirectory / filename); // The path the asset will write to
		}

		if (!asset->create(name)) {
			removeAsset(asset);
			return nullptr;
		}
		return static_cast<T*>(asset);
	}
}