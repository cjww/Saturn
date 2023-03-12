#pragma once

#include <filesystem>

#include <glm/vec4.hpp>
#include <glm/vec2.hpp>

#include "Renderer.hpp"
#include "structs.h"

#include "ProgressView.h"

#include "Tools/Profiler.h"

#include <Tools\utils.h>

#include "Assets\IAsset.h"
#include "Graphics/Material.h"
#include "Graphics\RenderTarget.h"


#define SA_ASSET_DIR "Assets/"

#define SA_DEFAULT_MATERIAL_NAME "default_material"
#define SA_DEFAULT_CUBE_NAME "default_cube"
#define SA_DEFAULT_QUAD_NAME "default_quad"
#define SA_DEFAULT_ASSET_DIR "Assets/default"


//--------------------------------------------------------------------------------------
// AssetManager is the class that will hold all assets. 
//--------------------------------------------------------------------------------------
namespace sa {

	class ModelAsset;
	
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

		std::filesystem::path m_defaultPath;

		AssetTypeID m_nextTypeID;
		std::unordered_map<AssetTypeID, std::function<IAsset* (const AssetHeader&)>> m_assetAddConversions;
		std::unordered_map<AssetTypeID, std::string> m_typeToString;
		std::unordered_map<std::string, AssetTypeID> m_stringToType;


		AssetManager();

		void locateAssetPackages();
		void locateStandaloneAssets();
		IAsset* addAsset(const std::filesystem::path& assetPath);

		void loadAssetPackage(AssetPackage& package);

		AssetPackage& newAssetPackage(const std::filesystem::path& path);

	public:
		~AssetManager();
	
		static AssetManager& get();
		
		static bool IsAsset(const std::filesystem::directory_entry& entry);

		void clear();

		Texture2D* loadDefaultTexture();
		Texture2D* loadDefaultBlackTexture();
		Texture2D* loadTexture(const std::filesystem::path& path, bool generateMipMaps);

		ModelAsset* loadQuad();
		ModelAsset* loadCube();

		const std::unordered_map<UUID, std::unique_ptr<IAsset>>& getAssets() const;
		void getAssets(std::vector<IAsset*>* assets, const std::string& filter) const;
		void getAssets(std::vector<IAsset*>* assets, AssetTypeID typeFilter) const;

		void rescanAssets();

		template<typename T>
		AssetTypeID registerAssetType();
		void getRegisteredAssetTypes(std::vector<AssetTypeID>& types);

		const std::string& getAssetTypeName(AssetTypeID typeID) const;

		template<typename T>
		AssetTypeID getAssetTypeID() const;

		template<typename T>
		T* getAsset(UUID id) const;
		IAsset* getAsset(UUID id) const;

		template<typename T>
		T* findAssetByName(const std::string& name) const;
		IAsset* findAssetByName(const std::string& name) const;

		template<typename T>
		T* findAssetByPath(const std::filesystem::path& path) const;
		IAsset* findAssetByPath(const std::filesystem::path& path) const;


		template<typename T>
		T* importAsset(const std::filesystem::path& path, const std::filesystem::path& assetDirectory = SA_ASSET_DIR);

		template<typename T>
		T* createAsset(const std::string& name, const std::filesystem::path& assetDirectory = SA_ASSET_DIR);

		IAsset* importAsset(AssetTypeID type, const std::filesystem::path& path, const std::filesystem::path& assetDirectory = SA_ASSET_DIR);
		IAsset* createAsset(AssetTypeID type, const std::string& name, const std::filesystem::path& assetDirectory = SA_ASSET_DIR);


		void removeAsset(IAsset* asset);
		void removeAsset(UUID id);

	};

	template<typename T>
	inline AssetTypeID AssetManager::registerAssetType() {
		AssetTypeID id = m_nextTypeID++;
		
		m_assetAddConversions[id] = [&](const AssetHeader& header) {
			return m_assets.insert({ header.id, std::make_unique<T>(header) }).first->second.get();
		};

		std::string str = typeid(T).name();
		utils::stripTypeName(str);
		m_typeToString[id] = str;
		m_stringToType[str] = id;

		return id;
	}

	template<typename T>
	inline AssetTypeID AssetManager::getAssetTypeID() const {
		std::string str = typeid(T).name();
		utils::stripTypeName(str);
		if (!m_stringToType.count(str)) {
			return -1;
		}
		return m_stringToType.at(str);
	}

	template<typename T>
	inline T* AssetManager::getAsset(UUID id) const {
		if (!m_assets.count(id)) 
			return nullptr;
		return dynamic_cast<T*>(m_assets.at(id).get());
	}

	template<typename T>
	inline T* AssetManager::findAssetByName(const std::string& name) const {
		return dynamic_cast<T*>(findAssetByName(name));
	}

	template<typename T>
	inline T* AssetManager::findAssetByPath(const std::filesystem::path& path) const {
		return dynamic_cast<T*>(findAssetByPath(path));
	}

	template<typename T>
	inline T* AssetManager::importAsset(const std::filesystem::path& path, const std::filesystem::path& assetDirectory) {
		SA_DEBUG_LOG_INFO("Importing Asset", path);
		AssetHeader header; // generates new UUID
		header.type = getAssetTypeID<T>();
		assert(header.type != -1 && "Can not use unregistered type!");
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
		
		if (!asset->importFromFile(path, assetDirectory)) {
			std::lock_guard<std::mutex> lock(m_mutex);
			m_importedAssets.erase(path);
			removeAsset(asset);
			return nullptr;
		}
		SA_DEBUG_LOG_INFO("Finished Importing Asset", path);

		asset->write();

		return static_cast<T*>(asset);
	}
	
	template<typename T>
	inline T* AssetManager::createAsset(const std::string& name, const std::filesystem::path& assetDirectory) {
		AssetHeader header; // generates new UUID
		header.type = getAssetTypeID<T>();
		assert(header.type != -1 && "Can not use unregistered type!");
		SA_DEBUG_LOG_INFO("Creating ", getAssetTypeName(header.type), " ", name);


		m_mutex.lock();
		auto [it, success] = m_assets.insert({ header.id, std::make_unique<T>(header) });
		m_mutex.unlock();

		IAsset* asset = it->second.get();

		asset->create(name, assetDirectory);
		SA_DEBUG_LOG_INFO("Finished Creating ", getAssetTypeName(header.type), " ", name);
		asset->write();

		return static_cast<T*>(asset);
	}
}