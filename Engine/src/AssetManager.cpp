#include "pch.h"
#include "AssetManager.h"


#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/color4.h"

#include "assimp/ProgressHandler.hpp"

#include "Tools/Vector.h"


#include "Assets/ModelAsset.h"
#include "Assets/TextureAsset.h"
#include "Scene.h"
#include "Assets/MaterialShader.h"

#include "Core.h"
#include "Engine.h"

namespace sa {
	
	void AssetManager::locateAssetPackages() {
		
	}

	void AssetManager::locateStandaloneAssets() {
		std::filesystem::path path = std::filesystem::current_path();
		for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
			if (IsAsset(entry)) {
				addAsset(std::filesystem::proximate(entry.path()));
			}
		}
	}

	Asset* AssetManager::addAsset(const std::filesystem::path& assetPath) {
		// read header
		std::ifstream file(assetPath, std::ios::binary);
		if (!file.good()) {
			SA_DEBUG_LOG_ERROR("Failed to open persumed asset ", assetPath);
			return nullptr;
		}
		AssetHeader header = Asset::readHeader(file);
		file.close();
		if (header.version != SA_ASSET_VERSION) {
			SA_DEBUG_LOG_WARNING("Asset versions do not match! ", assetPath, " (", header.version, " vs ", SA_ASSET_VERSION, ")");
			header.version = SA_ASSET_VERSION;
		}
		
		if (m_assets.count(header.id)) { // already loaded
			m_assets.at(header.id)->setAssetPath(assetPath); // Update Asset Path 
			return nullptr;
		}
		if (!m_assetAddConversions.count(header.type)) {
			SA_DEBUG_LOG_ERROR("Unknown type");
			return nullptr;
		}

		Asset* pAsset = m_assetAddConversions.at(header.type)(header);

		pAsset->setAssetPath(assetPath); // The path the asset will write to
		
		return pAsset;
	}

	void AssetManager::loadAssetPackage(AssetPackage& package) {
	
	}

	AssetPackage& AssetManager::newAssetPackage(const std::filesystem::path& path) {
		AssetPackage& package = m_assetPackages.emplace_back();
		package.path = path;
		package.file.open(path, std::ios::binary | std::ios::app);
		if (!package.file.is_open())
			SA_DEBUG_LOG_ERROR("Failed to open package file");
		return package;
	}


	AssetManager& AssetManager::get() {
		static AssetManager instance;
		return instance;
	}

	bool AssetManager::IsAsset(const std::filesystem::directory_entry& entry) {
		return entry.is_regular_file() && entry.path().extension() == ".asset";
	}


	void AssetManager::clear() {
		Asset::waitAllAssets();
		for (auto& [id, pAsset] : m_assets) {
			while (!pAsset->release());
		}
		m_assets.clear();
		m_assetPackages.clear();
	}
	
	Texture2D* AssetManager::loadDefaultTexture() {
		SA_PROFILE_FUNCTION();

		Texture2D* tex = ResourceManager::get().get<Texture2D>("default_white");
		if (tex)
			return tex;

		sa::Image img(2, 2, sa::Color{ 1, 1, 1, 1 });
		ResourceID id = ResourceManager::get().insert<Texture2D>("default_white", Texture2D(img, false));
		return ResourceManager::get().get<Texture2D>(id);
	}

	Texture2D* AssetManager::loadDefaultBlackTexture() {
		SA_PROFILE_FUNCTION();

		Texture2D* tex = ResourceManager::get().get<Texture2D>("default_black");
		if (tex)
			return tex;

		sa::Image img(2, 2, sa::Color{ 0, 0, 0, 0 });
		ResourceID id = ResourceManager::get().insert<Texture2D>("default_black", Texture2D(img, false));
		return ResourceManager::get().get<Texture2D>(id);
	}

	Texture2D* AssetManager::loadTexture(const std::filesystem::path& path, bool generateMipMaps) {
		SA_PROFILE_FUNCTION();
		ResourceID id = ResourceManager::get().keyToID<Texture2D>(path.generic_string());
		if (m_textures.count(id)) {
			return m_textures.at(id);
		}

		Texture2D* tex = ResourceManager::get().get<Texture2D>(id);

		if (!tex) {
			try {
				Image img(path.generic_string());
				id = ResourceManager::get().insert<Texture2D>(path.generic_string(), Texture2D(img, generateMipMaps));
			}
			catch (const std::exception& e) {
				return nullptr;
			}

			Texture2D* tex = ResourceManager::get().get<Texture2D>(id);
			m_textures[id] = tex;
			return tex;
		}
		return tex;
	}

	ModelAsset* AssetManager::loadQuad() {
		
		ModelAsset* pAsset = findAssetByName<ModelAsset>(SA_DEFAULT_QUAD_NAME);
		if (pAsset) {
			pAsset->load();
			return pAsset;
		}

		std::filesystem::create_directories(SA_DEFAULT_ASSET_DIR);
		pAsset = createAsset<ModelAsset>(SA_DEFAULT_QUAD_NAME, SA_DEFAULT_ASSET_DIR);
		
		Mesh mesh = {};
		
		mesh.vertices = {
			{ glm::vec4(-0.5f, 0.5f, 0, 1), glm::vec4(0, 0, 1, 0), glm::vec2(0, 0) },
			{ glm::vec4(0.5f, 0.5f, 0, 1), glm::vec4(0, 0, 1, 0), glm::vec2(1, 0) },
			{ glm::vec4(0.5f, -0.5f, 0, 1), glm::vec4(0, 0, 1, 0), glm::vec2(1, 1) },
			{ glm::vec4(-0.5f, -0.5f, 0, 1), glm::vec4(0, 0, 1, 0), glm::vec2(0, 1) }
		};

		mesh.indices = {
			0, 1, 3,
			1, 2, 3
		};
		
		Material* pMaterial = getDefaultMaterial();
		pMaterial->load();
		mesh.materialID = pMaterial->getID();

		pAsset->data.meshes.push_back(mesh);

		pAsset->load();
		pAsset->write();
		return pAsset;
	}

	ModelAsset* AssetManager::loadCube() {
		SA_PROFILE_FUNCTION();

		ModelAsset* pAsset = findAssetByName<ModelAsset>(SA_DEFAULT_CUBE_NAME);
		if (pAsset) {
			pAsset->load();
			return pAsset;
		}

		std::filesystem::create_directories(SA_DEFAULT_ASSET_DIR);
		pAsset = createAsset<ModelAsset>(SA_DEFAULT_CUBE_NAME, SA_DEFAULT_ASSET_DIR);

		Mesh& mesh = pAsset->data.meshes.emplace_back();

		mesh.vertices = {
			{ glm::vec4(-0.5f, 0.5f, 0.5f, 1), glm::vec4(0, 0, 1, 0), glm::vec2(0, 0) },
			{ glm::vec4(0.5f, 0.5f, 0.5f, 1), glm::vec4(0, 0, 1, 0), glm::vec2(1, 0) },
			{ glm::vec4(0.5f, -0.5f, 0.5f, 1), glm::vec4(0, 0, 1, 0), glm::vec2(1, 1) },
			{ glm::vec4(-0.5f, -0.5f, 0.5f, 1), glm::vec4(0, 0, 1, 0), glm::vec2(0, 1) },
			
			{ glm::vec4(0.5f, 0.5f, -0.5f, 1), glm::vec4(0, 0, -1, 0), glm::vec2(0, 0) },
			{ glm::vec4(-0.5f, 0.5f, -0.5f, 1), glm::vec4(0, 0, -1, 0), glm::vec2(1, 0) },
			{ glm::vec4(-0.5f, -0.5f, -0.5f, 1), glm::vec4(0, 0, -1, 0), glm::vec2(1, 1) },
			{ glm::vec4(0.5f, -0.5f, -0.5f, 1), glm::vec4(0, 0, -1, 0), glm::vec2(0, 1) },

			{ glm::vec4(0.5f, 0.5f, 0.5f, 1), glm::vec4(1, 0, 0, 0), glm::vec2(0, 0) },
			{ glm::vec4(0.5f, 0.5f, -0.5f, 1), glm::vec4(1, 0, 0, 0), glm::vec2(1, 0) },
			{ glm::vec4(0.5f, -0.5f, -0.5f, 1), glm::vec4(1, 0, 0, 0), glm::vec2(1, 1) },
			{ glm::vec4(0.5f, -0.5f, 0.5f, 1), glm::vec4(1, 0, 0, 0), glm::vec2(0, 1) },

			{ glm::vec4(-0.5f, 0.5f, -0.5f, 1), glm::vec4(-1, 0, 0, 0), glm::vec2(0, 0) },
			{ glm::vec4(-0.5f, 0.5f, 0.5f, 1), glm::vec4(-1, 0, 0, 0), glm::vec2(1, 0) },
			{ glm::vec4(-0.5f, -0.5f, 0.5f, 1), glm::vec4(-1, 0, 0, 0), glm::vec2(1, 1) },
			{ glm::vec4(-0.5f, -0.5f, -0.5f, 1), glm::vec4(-1, 0, 0, 0), glm::vec2(0, 1) },

			{ glm::vec4(-0.5f, -0.5f, 0.5f, 1), glm::vec4(0, -1, 0, 0), glm::vec2(0, 0) },
			{ glm::vec4(0.5f, -0.5f, 0.5f, 1), glm::vec4(0, -1, 0, 0), glm::vec2(1, 0) },
			{ glm::vec4(0.5f, -0.5f, -0.5f, 1), glm::vec4(0, -1, 0, 0), glm::vec2(1, 1) },
			{ glm::vec4(-0.5f, -0.5f, -0.5f, 1), glm::vec4(0, -1, 0, 0), glm::vec2(0, 1) },

			{ glm::vec4(0.5f, 0.5f, 0.5f, 1), glm::vec4(0, 1, 0, 0), glm::vec2(0, 0) },
			{ glm::vec4(-0.5f, 0.5f, 0.5f, 1), glm::vec4(0, 1, 0, 0), glm::vec2(1, 0) },
			{ glm::vec4(-0.5f, 0.5f, -0.5f, 1), glm::vec4(0, 1, 0, 0), glm::vec2(1, 1) },
			{ glm::vec4(0.5f, 0.5f, -0.5f, 1), glm::vec4(0, 1, 0, 0), glm::vec2(0, 1) },


		};

		mesh.indices = {
			0, 1, 3,
			1, 2, 3,

			4, 5, 7,
			5, 6, 7,

			8, 9, 11,
			9, 10, 11,

			12, 13, 15,
			13, 14, 15,

			16, 17, 19,
			17, 18, 19,
			
			20, 21, 23,
			21, 22, 23,
		};


		Material* pMaterial = getDefaultMaterial();
		pMaterial->load();
		mesh.materialID = pMaterial->getID();
		pAsset->load();
		pAsset->write();
		return pAsset;
	}

	Material* AssetManager::getDefaultMaterial() {
		Material* pAsset = findAssetByName<Material>(SA_DEFAULT_MATERIAL_NAME);
		if (pAsset) {
			return pAsset;
		}

		pAsset = createAsset<Material>(SA_DEFAULT_MATERIAL_NAME, SA_DEFAULT_ASSET_DIR);
		pAsset->setMaterialShader(getDefaultMaterialShader());
		pAsset->write();
		return pAsset;
	}

	MaterialShader* AssetManager::loadDefaultMaterialShader() {
		MaterialShader* pAsset = findAssetByName<MaterialShader>(SA_DEFAULT_MATERIAL_SHADER_NAME);
		if (pAsset) {
			pAsset->load();
			return pAsset;
		}

		std::filesystem::create_directories(SA_DEFAULT_ASSET_DIR);
		pAsset = createAsset<MaterialShader>(SA_DEFAULT_MATERIAL_SHADER_NAME, SA_DEFAULT_ASSET_DIR);
		
		auto vertexCode = ReadSPVFile((Engine::getShaderDirectory() / "ForwardPlusColorPass.vert.spv").generic_string().c_str());
		auto fragmentCode = ReadSPVFile((Engine::getShaderDirectory() / "ForwardPlusColorPass.frag.spv").generic_string().c_str());
		pAsset->create({ vertexCode, fragmentCode });

		pAsset->load(); // just to increase reference count
		pAsset->write();

		return pAsset;
	}

	MaterialShader* AssetManager::getDefaultMaterialShader() {
		MaterialShader* pAsset = findAssetByName<MaterialShader>(SA_DEFAULT_MATERIAL_SHADER_NAME);
		if (pAsset) {
			return pAsset;
		}

		std::filesystem::create_directories(SA_DEFAULT_ASSET_DIR);
		pAsset = createAsset<MaterialShader>(SA_DEFAULT_MATERIAL_SHADER_NAME, SA_DEFAULT_ASSET_DIR);

		auto vertexCode = ReadSPVFile((Engine::getShaderDirectory() / "ForwardPlusColorPass.vert.spv").generic_string().c_str());
		auto fragmentCode = ReadSPVFile((Engine::getShaderDirectory() / "ForwardPlusColorPass.frag.spv").generic_string().c_str());
		pAsset->create({ vertexCode, fragmentCode });
		
		pAsset->write();

		return pAsset;
	}

	const std::unordered_map<UUID, std::unique_ptr<Asset>>& AssetManager::getAssets() const{
		return m_assets;
	}

	void AssetManager::getAssets(std::vector<Asset*>* assets, const std::string& filter) const {
		for (auto& [id, asset] : m_assets) {
			if (asset->getName().find(filter) != std::string::npos) {
				assets->push_back(asset.get());
			}
		}
	}

	void AssetManager::getAssets(std::vector<Asset*>* assets, AssetTypeID typeFilter) const {
		for (auto& [id, asset] : m_assets) {
			if (asset->getType() == typeFilter) {
				assets->push_back(asset.get());
			}
		}
	}


	void AssetManager::rescanAssets() {
		locateStandaloneAssets();
		locateAssetPackages();
		for (auto& pkg : m_assetPackages) {
			loadAssetPackage(pkg);
		}
	}

	void AssetManager::getRegisteredAssetTypes(std::vector<AssetTypeID>& types) {
		types.clear();
		for (const auto& [id, _] : m_typeToString) {
			types.push_back(id);
		}
	}

	const std::string& AssetManager::getAssetTypeName(AssetTypeID typeID) const {
		if (!m_typeToString.count(typeID))
			return "Unknown";
		return m_typeToString.at(typeID);
	}

	Asset* AssetManager::getAsset(UUID id) const {
		if (!m_assets.count(id))
			return nullptr;
		return m_assets.at(id).get();
	}

	Asset* AssetManager::findAssetByName(const std::string& name) const {
		for (auto& [id, asset] : m_assets) {
			if (asset->getName() == name)
				return asset.get();
		}
		return nullptr;
	}

	Asset* AssetManager::findAssetByPath(const std::filesystem::path& path) const {
		if (!std::filesystem::exists(path))
			return nullptr;
		for (auto& [id, asset] : m_assets) {
			if (!std::filesystem::exists(asset->getAssetPath()))
				continue;
			if (std::filesystem::equivalent(asset->getAssetPath(), path))
				return asset.get();
		}
		return nullptr;
	}

	Asset* AssetManager::importAsset(AssetTypeID type, const std::filesystem::path& path, const std::filesystem::path& assetDirectory) {
		SA_DEBUG_LOG_INFO("Importing ", getAssetTypeName(type), " ", path);
		AssetHeader header; // generates new UUID
		header.type = type;
		assert(header.type != -1 && "Can not use unregistered type!");
		Asset* asset;
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			if (m_importedAssets.count(path)) {
				return m_assets.at(m_importedAssets.at(path)).get();
			}
			m_importedAssets[path] = header.id;
			asset = m_assetAddConversions[type](header);
		}

		if (!asset->importFromFile(path, assetDirectory)) {
			std::lock_guard<std::mutex> lock(m_mutex);
			m_importedAssets.erase(path);
			removeAsset(asset);
			return nullptr;
		}
		SA_DEBUG_LOG_INFO("Finished Importing ", getAssetTypeName(type), " ", path);

		asset->write();

		return asset;
	}


	Asset* AssetManager::createAsset(AssetTypeID type, const std::string& name, const std::filesystem::path& assetDirectory) {
		SA_DEBUG_LOG_INFO("Creating ", getAssetTypeName(type), " ", name);

		AssetHeader header; // generates new UUID
		header.type = type;
		assert(header.type != -1 && "Can not use unregistered type!");

		m_mutex.lock();
		Asset* asset = m_assetAddConversions[type](header);
		m_mutex.unlock();


		asset->create(name, assetDirectory);
		SA_DEBUG_LOG_INFO("Finished Creating ", getAssetTypeName(type), " ", name);
		asset->write();

		return asset;
	}


	void AssetManager::removeAsset(Asset* asset) {
		m_assets.erase(asset->getID());
	}

	void AssetManager::removeAsset(UUID id) {
		m_assets.erase(id);
	}

	AssetManager::AssetManager() {
		loadDefaultTexture();
		sa::ResourceManager::get().setCleanupFunction<Texture2D>([](Texture2D* pTexture) {
			pTexture->destroy();
		});

		m_nextTypeID = 0;

		Asset::reg();
		registerAssetType<ModelAsset>();
		registerAssetType<Material>();
		registerAssetType<TextureAsset>();
		registerAssetType<Scene>();
		registerAssetType<RenderTarget>();
		registerAssetType<MaterialShader>();
	
	}

	AssetManager::~AssetManager() {
		ResourceManager::get().clearContainer<Texture2D>();
	}
}