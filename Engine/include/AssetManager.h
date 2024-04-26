#pragma once

#include <filesystem>
#include <mutex>

#include <glm/vec4.hpp>
#include <glm/vec2.hpp>

#include "Renderer.hpp"
#include "structs.h"

#include "ProgressView.h"

#include "Tools/Profiler.h"

#include <Tools\utils.h>

#include "Assets\Asset.h"

#include "Lua/LuaAccessable.h"

#define SA_ASSET_DIR "Assets/"

#define SA_DEFAULT_MATERIAL_SHADER_NAME "default_material_shader"
#define SA_DEFAULT_MATERIAL_SHADER_ID 1ull

#define SA_DEFAULT_MATERIAL_NAME "default_material"
#define SA_DEFAULT_MATERIAL_ID 2ull

#define SA_BUILTIN_CUBE_NAME "builtin_cube"
#define SA_BUILTIN_QUAD_NAME "builtin_quad"

#define SA_BUILTIN_ASSET_DIR "Assets/builtin"



//--------------------------------------------------------------------------------------
// AssetManager is the class that will hold all assets. 
//--------------------------------------------------------------------------------------
namespace sa {

	class ModelAsset;
	class Material;
	class TextureAsset;
	class MaterialShader;

	typedef uint32_t AssetPackageFlags;
	enum class AssetPackageFlagBits : AssetPackageFlags {
		
	};

	struct AssetPackageHeader {
		size_t assetCount;
		AssetPackageFlags flags;
	};

	// Singelton class
	class AssetManager {
	private:	
		mutable std::mutex m_mutex;

		std::unordered_map<ResourceID, Texture*> m_textures;

		std::unordered_map<UUID, std::filesystem::path> m_importedAssets;

		std::unordered_map<UUID, std::unique_ptr<Asset>> m_assets;

		AssetTypeID m_nextTypeID;
		std::unordered_map<AssetTypeID, std::function<Asset* (const AssetHeader&, bool)>> m_assetAddConversions;
		std::unordered_map<AssetTypeID, std::string> m_typeToString;
		std::unordered_map<std::string, AssetTypeID> m_stringToType;

		std::unordered_map<std::string, AssetTypeID> m_extensionToType;


		AssetManager();

		void locateAssets();
		Asset* addAsset(const std::filesystem::path& assetPath);
		Asset* addAsset(AssetHeader header, const std::filesystem::path& assetPath, bool isCompiled);

		
		void addAssetPackage(const std::filesystem::path& packagePath);

		template<typename T>
		T* createAsset(const std::string& name, UUID id);


	public:

		~AssetManager();
	
		static AssetManager& Get();
		
		static bool IsMetaAsset(const std::filesystem::directory_entry& entry);
		static bool IsCompiledAsset(const std::filesystem::directory_entry& entry);
		static bool IsAssetSource(const std::filesystem::directory_entry& entry);
		static bool IsAssetPackage(const std::filesystem::directory_entry& entry);

		AssetTypeID getAssetTypeByFile(const std::filesystem::path& path) const;
		

		void clear();

		Texture* loadDefaultTexture();
		Texture* loadDefaultBlackTexture();
		Texture* loadTexture(const std::filesystem::path& path, bool generateMipMaps);

		ModelAsset* getQuad();
		ModelAsset* getCube();

		Material* getDefaultMaterial();
		MaterialShader* getDefaultMaterialShader();


		const std::unordered_map<UUID, std::unique_ptr<Asset>>& getAssets() const;
		void getAssets(std::vector<Asset*>* assets, const std::string& filter) const;
		void getAssets(std::vector<Asset*>* assets, AssetTypeID typeFilter) const;

		void getAssets(std::vector<UUID>* assets, AssetTypeID typeFilter) const;

		void rescanAssets();

		template<typename T>
		AssetTypeID registerAssetType();
		void getRegisteredAssetTypes(std::vector<AssetTypeID>& types);

		const std::string& getAssetTypeName(AssetTypeID typeID) const;

		template<typename T>
		AssetTypeID getAssetTypeID() const;

		template<typename T>
		bool isType(Asset* pAsset) const;

		template<typename T>
		T* getAsset(UUID id) const;
		Asset* getAsset(UUID id) const;

		template<typename T>
		T* findAssetByName(const std::string& name) const;
		Asset* findAssetByName(const std::string& name) const;

		template<typename T>
		T* findAssetByPath(const std::filesystem::path& path) const;
		Asset* findAssetByPath(const std::filesystem::path& path) const;


		template<typename T>
		T* importAsset(const std::filesystem::path& path, const std::filesystem::path& assetDirectory = SA_ASSET_DIR);

		bool wasImported(Asset* pAsset) const;
		void reimportAsset(Asset* pAsset);


		template<typename T>
		T* createAsset(const std::string& name, const std::filesystem::path& assetDirectory = SA_ASSET_DIR);

		Asset* importAsset(AssetTypeID type, const std::filesystem::path& path, const std::filesystem::path& assetDirectory = SA_ASSET_DIR);
		Asset* createAsset(AssetTypeID type, const std::string& name, const std::filesystem::path& assetDirectory = SA_ASSET_DIR);

		void makeAssetPackage(const std::vector<UUID>& assets, const std::filesystem::path& packagePath);

		void removeAsset(Asset* asset);
		void removeAsset(UUID id);

		bool eraseAsset(Asset* asset);
		bool eraseAsset(UUID id);


	};

	template <typename T>
	T* AssetManager::createAsset(const std::string& name, UUID id) {
		AssetHeader header; // generates new UUID
		header.id = id;
		header.type = getAssetTypeID<T>();
		assert(header.type != -1 && "Can not use unregistered type!");
		SA_DEBUG_LOG_INFO("Creating ", getAssetTypeName(header.type), " ", name, " with id ", std::to_string(id));

		auto [it, success] = m_assets.insert({ header.id, std::make_unique<T>(header, true) });

		Asset* asset = it->second.get();

		asset->create(name, "");
		SA_DEBUG_LOG_INFO("Finished Creating ", getAssetTypeName(header.type), " ", name);
		return static_cast<T*>(asset);
	}

	template<typename T>
	inline AssetTypeID AssetManager::registerAssetType() {
		AssetTypeID id = m_nextTypeID++;
		
		m_assetAddConversions[id] = [&](const AssetHeader& header, bool isCompiled) {
			return m_assets.insert({ header.id, std::make_unique<T>(header, isCompiled) }).first->second.get();
		};

		std::string str = typeid(T).name();
		utils::stripTypeName(str);
		m_typeToString[id] = str;
		m_stringToType[str] = id;

		LuaAccessable::registerType<T>();
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
	inline bool AssetManager::isType(Asset* pAsset) const {
		return pAsset && pAsset->getHeader().type == getAssetTypeID<T>();
	}

	template<typename T>
	inline T* AssetManager::getAsset(UUID id) const {
		sa::Asset* pAsset = getAsset(id);
		if (!isType<T>(pAsset))
			return nullptr;
		return static_cast<T*>(pAsset);
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
		if (Asset* pAsset = importAsset(getAssetTypeID<T>(), path, assetDirectory))
			return static_cast<T*>(pAsset);
		return nullptr;
	}
	
	template<typename T>
	inline T* AssetManager::createAsset(const std::string& name, const std::filesystem::path& assetDirectory) {
		AssetHeader header; // generates new UUID
		header.type = getAssetTypeID<T>();
		assert(header.type != -1 && "Can not use unregistered type!");
		SA_DEBUG_LOG_INFO("Creating ", getAssetTypeName(header.type), " ", name);

		auto [it, success] = m_assets.insert({ header.id, std::make_unique<T>(header, true) });

		Asset* asset = it->second.get();

		asset->create(name, assetDirectory);
		SA_DEBUG_LOG_INFO("Finished Creating ", getAssetTypeName(header.type), " ", name);
		return static_cast<T*>(asset);
	}

}
