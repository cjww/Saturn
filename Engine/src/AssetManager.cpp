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

#include "Lua\LuaTypes.h"

namespace sa {
	
	void AssetManager::locateAssets() {
		std::filesystem::path path = std::filesystem::current_path();
		for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
			if (IsAsset(entry)) {
				addAsset(std::filesystem::proximate(entry.path()));
			}
			else if(IsAssetPackage(entry)) {
				addAssetPackage(std::filesystem::proximate(entry.path()));
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
		AssetHeader header = Asset::ReadHeader(file);
		file.close();
		return addAsset(header, assetPath);
	}

	Asset* AssetManager::addAsset(AssetHeader header, const std::filesystem::path& assetPath) {
		if (header.version != SA_ASSET_VERSION) {
			SA_DEBUG_LOG_WARNING("Asset versions do not match! ", assetPath, " (", header.version, " vs ", SA_ASSET_VERSION, ")");
			header.version = SA_ASSET_VERSION;
		}

		if (m_assets.count(header.id)) { // already loaded
			m_assets.at(header.id)->setAssetPath(assetPath); // Update Asset Path
			m_assets.at(header.id)->setHeader(header); // And header to bring over correct content offset value
			return nullptr;
		}
		if (!m_assetAddConversions.count(header.type)) {
			throw std::runtime_error("Unknown Asset Type " + std::to_string(header.type));
		}

		Asset* pAsset = m_assetAddConversions.at(header.type)(header);

		pAsset->setAssetPath(assetPath); // The path the asset will write to

		return pAsset;
	}

	void AssetManager::addAssetPackage(const std::filesystem::path& packagePath) {
		std::ifstream file(packagePath, std::ios::binary);
		if (!file.good()) {
			SA_DEBUG_LOG_ERROR("Failed to open persumed asset package ", packagePath);
			return;
		}

		AssetPackageHeader header = {};
		file.read(reinterpret_cast<char*>(&header), sizeof(header));
		SA_DEBUG_LOG_INFO("Package contains ", header.assetCount, " assets");
		for(size_t i = 0; i < header.assetCount; i++) {
			AssetHeader assetHeader = Asset::ReadHeader(file);
			addAsset(assetHeader, packagePath);
			SA_DEBUG_LOG_INFO(i, ": Asset size: ", assetHeader.size, " - Asset content offset: ", assetHeader.contentOffset, " - Asset Type: ", getAssetTypeName(assetHeader.type));
		}

		file.close();
	}

	AssetManager& AssetManager::get() {
		static AssetManager instance;
		return instance;
	}

	bool AssetManager::IsAsset(const std::filesystem::directory_entry& entry) {
		return entry.is_regular_file() && entry.path().extension() == SA_ASSET_EXTENSION;
	}

	bool AssetManager::IsAssetPackage(const std::filesystem::directory_entry& entry) {
		return entry.is_regular_file() && entry.path().extension() == SA_ASSET_PACKAGE_EXTENSION;
	}


	void AssetManager::clear() {
		Asset::waitAllAssets();
		for (auto& [id, pAsset] : m_assets) {
			while (!pAsset->release());
		}
		m_assets.clear();
	}
	
	Texture* AssetManager::loadDefaultTexture() {
		SA_PROFILE_FUNCTION();

		Texture* pTex = ResourceManager::get().get<Texture>("default_white");
		if (pTex)
			return pTex;

		sa::Image img(2, 2, sa::Color{ 1, 1, 1, 1 });
		Texture texture;
		texture.create2D(img, false);
		ResourceID id = ResourceManager::get().insert<Texture>("default_white", texture);
		return ResourceManager::get().get<Texture>(id);
	}

	Texture* AssetManager::loadDefaultBlackTexture() {
		SA_PROFILE_FUNCTION();

		Texture* tex = ResourceManager::get().get<Texture>("default_black");
		if (tex)
			return tex;

		sa::Image img(2, 2, sa::Color{ 0, 0, 0, 0 });
		Texture texture;
		texture.create2D(img, false);
		ResourceID id = ResourceManager::get().insert<Texture>("default_black", texture);
		return ResourceManager::get().get<Texture>(id);
	}

	Texture* AssetManager::loadTexture(const std::filesystem::path& path, bool generateMipMaps) {
		SA_PROFILE_FUNCTION();
		ResourceID id = ResourceManager::get().keyToID<Texture>(path.generic_string());
		if (m_textures.count(id)) {
			return m_textures.at(id);
		}

		Texture* tex = ResourceManager::get().get<Texture>(id);

		if (!tex) {
			try {
				Image img(path.generic_string());
				Texture texture;
				texture.create2D(img, generateMipMaps);
				id = ResourceManager::get().insert<Texture>(path.generic_string(), texture);
			}
			catch (const std::exception& e) {
				return nullptr;
			}

			Texture* tex = ResourceManager::get().get<Texture>(id);
			m_textures[id] = tex;
			return tex;
		}
		return tex;
	}

	ModelAsset* AssetManager::getQuad() {
		
		ModelAsset* pAsset = findAssetByName<ModelAsset>(SA_BUILTIN_QUAD_NAME);
		if (pAsset) {
			return pAsset;
		}

		std::filesystem::create_directories(SA_BUILTIN_ASSET_DIR);
		pAsset = createAsset<ModelAsset>(SA_BUILTIN_QUAD_NAME, SA_BUILTIN_ASSET_DIR);
		
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
		
		mesh.material = getDefaultMaterial();

		pAsset->data.meshes.push_back(mesh);

		pAsset->write();
		return pAsset;
	}

	ModelAsset* AssetManager::getCube() {
		SA_PROFILE_FUNCTION();

		ModelAsset* pAsset = findAssetByName<ModelAsset>(SA_BUILTIN_CUBE_NAME);
		if (pAsset) {
			return pAsset;
		}

		std::filesystem::create_directories(SA_BUILTIN_ASSET_DIR);
		pAsset = createAsset<ModelAsset>(SA_BUILTIN_CUBE_NAME, SA_BUILTIN_ASSET_DIR);

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


		mesh.material = getDefaultMaterial();
		pAsset->write();
		return pAsset;
	}

	Material* AssetManager::getDefaultMaterial() {
		std::lock_guard lock(m_mutex);
		Material* pAsset = getAsset<Material>(SA_DEFAULT_MATERIAL_ID);
		if (pAsset) {
			return pAsset;
		}
		pAsset = createAsset<Material>(SA_DEFAULT_MATERIAL_NAME, SA_DEFAULT_MATERIAL_ID);
		pAsset->hold(); // Make sure this is not unloaded, because it can't be loaded again
		return pAsset;
	}

	MaterialShader* AssetManager::getDefaultMaterialShader() {
		std::lock_guard lock(m_mutex);
		MaterialShader* pMaterialShader = getAsset<MaterialShader>(SA_DEFAULT_MATERIAL_SHADER_ID);
		if (pMaterialShader) {
			return pMaterialShader;
		}

		pMaterialShader = createAsset<MaterialShader>(SA_DEFAULT_MATERIAL_SHADER_NAME, SA_DEFAULT_MATERIAL_SHADER_ID);

		auto vertexCode = ReadSPVFile((Engine::getShaderDirectory() / "ForwardPlusColorPass.vert.spv").generic_string().c_str());
		auto fragmentCode = ReadSPVFile((Engine::getShaderDirectory() / "ForwardPlusColorPass.frag.spv").generic_string().c_str());
		pMaterialShader->create({ vertexCode, fragmentCode });

		pMaterialShader->hold(); // Make sure this is not unloaded, because it can't be loaded again

		return pMaterialShader;
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

	void AssetManager::getAssets(std::vector<UUID>* assets, AssetTypeID typeFilter) const {
		for (auto& [id, asset] : m_assets) {
			if (asset->getType() == typeFilter) {
				assets->push_back(id);
			}
		}
	}

	void AssetManager::rescanAssets() {
		locateAssets();
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
		std::lock_guard lock(m_mutex);
		for (auto& [id, asset] : m_assets) {
			if (asset->getName() == name)
				return asset.get();
		}
		return nullptr;
	}

	Asset* AssetManager::findAssetByPath(const std::filesystem::path& path) const {
		std::lock_guard lock(m_mutex);
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

	bool AssetManager::wasImported(Asset* pAsset) const {
		return m_importedAssets.count(pAsset->getID());
	}

	void AssetManager::reimportAsset(Asset* pAsset) {
		m_mutex.lock();
		if (!m_importedAssets.count(pAsset->getID())) {
			SA_DEBUG_LOG_WARNING("Asset was never imported: ", pAsset->getName());
			m_mutex.unlock();
			return;
		}
		auto& path = m_importedAssets.at(pAsset->getID());
		m_mutex.unlock();

		if (!pAsset->importFromFile(path, pAsset->getAssetPath().parent_path())) {

		}

	}

	Asset* AssetManager::importAsset(AssetTypeID type, const std::filesystem::path& path, const std::filesystem::path& assetDirectory) {
		SA_DEBUG_LOG_INFO("Importing ", getAssetTypeName(type), " ", path);
		AssetHeader header; // generates new UUID
		header.type = type;
		assert(header.type != -1 && "Can not use unregistered type!");
		Asset* asset;
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			
			m_importedAssets[header.id] = path;
			asset = m_assetAddConversions[type](header);
		}

		if (!asset->importFromFile(path, assetDirectory)) {
			std::lock_guard<std::mutex> lock(m_mutex);
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

	void AssetManager::makeAssetPackage(const std::vector<UUID>& assets, const std::filesystem::path& packagePath) {


		// make sure all assets are loaded and not unloaded while writing
		std::vector<AssetHolder<Asset>> heldAssets;
		heldAssets.reserve(assets.size());
		for (auto& id : assets) {
			Asset* pAsset = getAsset(id);
			heldAssets.emplace_back(pAsset);
		}
		for (auto& asset : heldAssets) {
			asset.getProgress()->waitAll();
		}

		std::ofstream file;
		if(packagePath.extension() == SA_ASSET_PACKAGE_EXTENSION) {
			file.open(packagePath, std::ios::binary);
		}
		else {
			auto path = packagePath;
			path.replace_extension(SA_ASSET_PACKAGE_EXTENSION);
			file.open(path, std::ios::binary);
		}

		if(!file.good()) {
			SA_DEBUG_LOG_ERROR("Failed to open file for writing: ", packagePath);
			return;
		}

		AssetPackageHeader packageHeader = {};
		packageHeader.assetCount = assets.size();
		packageHeader.flags = 0;
		file.write(reinterpret_cast<const char*>(&packageHeader), sizeof(packageHeader));

		std::streampos headerPos = file.tellp();
		std::streampos contentPos = headerPos;
		contentPos += sizeof(AssetHeader) * assets.size();

		for(auto& asset : heldAssets) {
			Asset* pAsset = asset.getAsset();
			auto header = pAsset->getHeader();

			file.seekp(contentPos);
			header.contentOffset = contentPos;
			pAsset->onWrite(file, 0);
			header.size = file.tellp() - contentPos;
			contentPos = file.tellp();

			file.seekp(headerPos);
			Asset::WriteHeader(header, file);
			headerPos = file.tellp();

			if(pAsset->isFromPackage(packagePath))
				pAsset->setHeader(header);
		}

		file.close();
	}


	void AssetManager::removeAsset(Asset* asset) {
		removeAsset(asset->getID());
	}

	void AssetManager::removeAsset(UUID id) {
		m_assets.erase(id);
		m_importedAssets.erase(id);
	}

	bool AssetManager::eraseAsset(Asset* asset) {
		if (!asset)
			return false;
		if (asset->getReferenceCount() > 0)
			throw std::runtime_error("Can not erase asset "
				+ asset->getName()
				+ " ("
				+ std::to_string(asset->getID()) 
				+ "), as it is still being used - Ref Count: "
				+ std::to_string(asset->getReferenceCount()));

		const auto& path = asset->getAssetPath();
		if(!path.empty() && std::filesystem::exists(path)) {
			try {
				std::filesystem::remove(path); // delete file
			}
			catch(const std::filesystem::filesystem_error& e) {
				SA_DEBUG_LOG_ERROR("Failed to remove file: ", e.what(), " -> ", e.path1());
				return false;
			}
		}
		removeAsset(asset); // remove from memory
		return true;
	}

	bool AssetManager::eraseAsset(UUID id) {
		return eraseAsset(getAsset(id));
	}

	AssetManager::AssetManager() {
		loadDefaultTexture();
		sa::ResourceManager::get().setCleanupFunction<Texture>([](Texture* pTexture) {
			pTexture->destroy();
		});

		m_nextTypeID = 0;


		registerAssetType<ModelAsset>();
		registerAssetType<Material>();
		registerAssetType<TextureAsset>();
		registerAssetType<Scene>();
		registerAssetType<RenderTarget>();
		registerAssetType<MaterialShader>();
	
	}

	AssetManager::~AssetManager() {
		ResourceManager::get().clearContainer<Texture>();
	}
}