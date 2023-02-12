#include "pch.h"
#include "AssetManager.h"


#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/color4.h"

#include "assimp/ProgressHandler.hpp"

#include "Tools/Vector.h"


#include "Assets/ModelAsset.h"
#include "Assets/MaterialAsset.h"
#include "Assets/TextureAsset.h"
#include "Scene.h"

namespace sa {
	
	void AssetManager::locateAssetPackages() {
		
	}

	void AssetManager::locateStandaloneAssets() {
		std::filesystem::path path = std::filesystem::current_path();
		for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
			if (entry.is_regular_file()) {
				if (entry.path().extension() == ".asset") {
					addAsset(std::filesystem::proximate(entry.path()));
				}
			}
		}

	}

	IAsset* AssetManager::addAsset(const std::filesystem::path& assetPath) {
		// read header
		std::ifstream file(assetPath, std::ios::binary);
		if (!file.good()) {
			SA_DEBUG_LOG_ERROR("Failed to open persumed asset ", assetPath);
		}
		AssetHeader header = IAsset::readHeader(file);
		file.close();
	
		if (m_assets.count(header.id)) {
			return nullptr;
		}
		if (!m_assetAddConversions.count(header.type)) {
			SA_DEBUG_LOG_ERROR("Unknown type");
			return nullptr;
		}

		IAsset* pAsset = m_assetAddConversions.at(header.type)(header);

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

	void AssetManager::clear() {
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

	
	MaterialAsset* AssetManager::loadDefaultMaterial() {
		SA_PROFILE_FUNCTION();
		MaterialAsset* pAsset = getAsset<MaterialAsset>(m_defaultMaterial);
		if (pAsset)
			return pAsset;

		std::filesystem::create_directories(m_defaultPath);
		pAsset = createAsset<MaterialAsset>("Default Material", m_defaultPath);
		m_defaultMaterial = pAsset->getID();

		Material& mat = pAsset->data;
		memset(&mat.values, 0, sizeof(mat.values));
		mat.values.ambientColor = SA_COLOR_WHITE;
		mat.values.diffuseColor = SA_COLOR_WHITE;
		mat.values.specularColor = SA_COLOR_WHITE;
		mat.values.opacity = 1.0f;
		mat.values.shininess = 1.0f;
		mat.twoSided = false;

		return pAsset;
	}

	ModelAsset* AssetManager::loadQuad() {
		ModelAsset* pAsset = getAsset<ModelAsset>(m_quad);
		if (pAsset)
			return pAsset;

		std::filesystem::create_directories(m_defaultPath);
		pAsset = createAsset<ModelAsset>("Quad", m_defaultPath);
		m_quad = pAsset->getID();

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
		
		mesh.materialID = loadDefaultMaterial()->getID();

		pAsset->data.meshes.push_back(mesh);

		return pAsset;
	}

	ModelAsset* AssetManager::loadCube() {
		SA_PROFILE_FUNCTION();

		ModelAsset* pAsset = getAsset<ModelAsset>(m_cube);
		if (pAsset)
			return pAsset;


		std::filesystem::create_directories(m_defaultPath);
		pAsset = createAsset<ModelAsset>("Cube", m_defaultPath);
		m_cube = pAsset->getID();

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

		mesh.materialID = loadDefaultMaterial()->getID();
		return pAsset;
	}

	const std::unordered_map<UUID, std::unique_ptr<IAsset>>& AssetManager::getAssets() const{
		return m_assets;
	}

	void AssetManager::rescanAssets() {
		clear();

		locateStandaloneAssets();
		locateAssetPackages();
		for (auto& pkg : m_assetPackages) {
			loadAssetPackage(pkg);
		}
	}

	const std::string& AssetManager::getAssetTypeName(AssetTypeID typeID) const {
		if (!m_typeStrings.count(typeID))
			return "Unknown";
		return m_typeStrings.at(typeID);
	}

	IAsset* AssetManager::getAsset(UUID id) const {
		if (!m_assets.count(id))
			return nullptr;
		return m_assets.at(id).get();
	}

	void AssetManager::removeAsset(IAsset* asset) {
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
		m_defaultPath = "Assets/Default/";

		registerAssetType<ModelAsset>();
		registerAssetType<MaterialAsset>();
		registerAssetType<TextureAsset>();
		registerAssetType<Scene>();
	}

	AssetManager::~AssetManager() {
		ResourceManager::get().clearContainer<Texture2D>();
	}
}