#include "pch.h"
#include "AssetManager.h"


#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/color4.h"

#include "assimp/ProgressHandler.hpp"

#include "Tools/Logger.hpp"
#include "Tools/Vector.h"


namespace sa {

	bool searchForFile(const std::filesystem::path& directory, const std::filesystem::path& filename, std::filesystem::path& outPath) {
		outPath = directory / filename;
		if (std::filesystem::exists(outPath)) {
			return true;
		}

		std::filesystem::recursive_directory_iterator it(directory);
		DEBUG_LOG_INFO("Looking for file:", filename.filename());
		while(it != std::filesystem::end(it)) {
			
			if (it->path().filename() == filename.filename()) {
				outPath = it->path();
				DEBUG_LOG_INFO("Found in:", outPath);
				return true;
			}
			it++;
		}
		std::filesystem::path dir = directory;
		if (!dir.has_parent_path()) {
			dir = std::filesystem::absolute(dir).parent_path();
		}
		std::filesystem::recursive_directory_iterator parentIt(dir);
		
		while(parentIt != std::filesystem::end(parentIt)) {
			if (parentIt->path() == directory) {
				parentIt.disable_recursion_pending();
			}
			else if (parentIt->path().filename() == filename.filename()) {
				outPath = parentIt->path();
				DEBUG_LOG_INFO("Found in:", outPath);
				return true;
			}
			parentIt++;
			
		}

		return false;
	}

	void processNode(const aiScene* scene, const aiNode* node, ModelData* pModelData, sa::ProgressView<ResourceID> progress) {
		SA_PROFILE_FUNCTION();
		for (int i = 0; i < node->mNumMeshes; i++) {
			Mesh mesh = {};
			const aiMesh* aMesh = scene->mMeshes[node->mMeshes[i]];
			
			std::vector<VertexNormalUV> vertices;
			std::vector<uint32_t> indices;

			std::unordered_map<VertexNormalUV, uint32_t> vertexIndices;
			
			for (int j = 0; j < aMesh->mNumFaces; j++) {
				aiFace face = aMesh->mFaces[j];

				for (int k = face.mNumIndices - 1; k >= 0; k--) {
					sa::VertexNormalUV vertex = {};
					
					aiVector3D pos = aMesh->mVertices[face.mIndices[k]];
					vertex.position = { pos.x, pos.y, pos.z , 1.f };

					if (aMesh->HasTextureCoords(0)) {
						aiVector3D texCoord = aMesh->mTextureCoords[0][face.mIndices[k]]; // assumes 1 tex coord per vertex
						vertex.texCoord = { texCoord.x, texCoord.y };
					}

					if (aMesh->HasNormals()) {
						aiVector3D normal = aMesh->mNormals[face.mIndices[k]]; // assumes 1 tex coord per vertex
						vertex.normal = { normal.x, normal.y, normal.z, 0.f };
					}
					
					if (vertexIndices.count(vertex)) {
						indices.push_back(vertexIndices.at(vertex));
						continue;
					}

					vertices.push_back(vertex);
					vertexIndices[vertex] = vertices.size() - 1;
					indices.push_back(vertices.size() - 1);
				}
			}
			mesh.vertexBuffer = Renderer::get().createBuffer(sa::BufferType::VERTEX);
			mesh.vertexBuffer.write(vertices);

			mesh.indexBuffer = Renderer::get().createBuffer(sa::BufferType::INDEX);
			mesh.indexBuffer.write(indices);

			mesh.materialID = aMesh->mMaterialIndex;

			pModelData->meshes.push_back(mesh);
			progress.increment();
		}

		for (int i = 0; i < node->mNumChildren; i++) {
			processNode(scene, node->mChildren[i], pModelData, progress);
		}
	}

	void loadMaterialTexture(Material& material, const std::filesystem::path& directory, aiTextureType type, aiTexture** ppAiTextures, aiMaterial* pAiMaterial) {
		std::vector<BlendedTexture> textures(pAiMaterial->GetTextureCount(type));

		for (size_t i = 0; i < textures.size(); i++) {
			aiString str;
			ai_real blending = 1.0f;
			aiTextureOp op = aiTextureOp::aiTextureOp_Multiply;
			pAiMaterial->GetTexture(type, i, &str, NULL, NULL, &blending, &op);

			std::filesystem::path filename(str.C_Str());

			if (str.data[0] == '*') { // Texture is embeded
				int index = std::atoi(str.data + 1);
				aiTexture* pAiTex = ppAiTextures[index];
				if (pAiTex->mHeight == 0) { // texture is compressed
					filename = std::string(pAiTex->mFilename.data) + "." +  std::string(pAiTex->achFormatHint);
				}
				else { // uncompressed
					filename = std::string(pAiTex->mFilename.data); // read from file anyway
				}
			}
			std::filesystem::path finalPath;
			if (!searchForFile(directory, filename, finalPath)) {
				DEBUG_LOG_ERROR("File not found:", filename);
				continue;
			}

			Texture2D* tex = AssetManager::get().loadTexture(finalPath, true);
			if (!tex) {
				DEBUG_LOG_ERROR("Failed to create texture,", finalPath.string());
				continue;
			}

			//DEBUG_LOG_INFO("Image found:", finalPath.filename(), ", type:", toString((MaterialTextureType)type));
			textures[i].texture = *tex;
			textures[i].blendFactor = blending;
			textures[i].blendOp = (TextureBlendOp)op;
		
		}
		
		material.setTextures(textures, (MaterialTextureType)type);
	}

	sa::Color getColor(aiMaterial* pMaterial, const char* pKey, unsigned int type, unsigned int idx, sa::Color defaultColor = SA_COLOR_WHITE) {
		aiColor3D color = {};
		pMaterial->Get(pKey, type, idx, color);
		if (!color.IsBlack())
			return { color.r, color.g, color.b, 1 };
		return defaultColor;
	}

	AssetManager& AssetManager::get() {
		static AssetManager instance;
		return instance;
	}
	
	Texture2D* AssetManager::loadDefaultTexture() {
		Texture2D* tex = ResourceManager::get().get<Texture2D>("default_white");
		if (tex)
			return tex;

		sa::Image img(2, 2, sa::Color{ 1, 1, 1, 1 });
		ResourceID id = sa::ResourceManager::get().insert<Texture2D>("default_white", sa::Renderer::get().createTexture2D(img, false));
		return sa::ResourceManager::get().get<Texture2D>(id);
	}

	Texture2D* AssetManager::loadTexture(const std::filesystem::path& path, bool generateMipMaps) {
		SA_PROFILE_FUNCTION();
		Texture2D* tex = ResourceManager::get().get<Texture2D>(path.string());
		
		if (!tex) {
			try {
				Image img(path.string());
				ResourceManager::get().insert<Texture2D>(path.string(), Renderer::get().createTexture2D(img, generateMipMaps));
				DEBUG_LOG_INFO("Loaded texture", path.string());
			}
			catch (const std::exception& e) {
				return nullptr;
			}

			return ResourceManager::get().get<Texture2D>(path.string());
		}
		return tex;
	}

	std::tuple<ResourceID, ModelData*> AssetManager::newModel(const std::string& name) {
		ResourceID id;
		if(!name.empty())
			id = ResourceManager::get().insert<ModelData>(name, {});
		else
			id = ResourceManager::get().insert<ModelData>();
		return { id, ResourceManager::get().get<ModelData>(id) };
	}

	ResourceID AssetManager::loadModel(const std::filesystem::path& path, ProgressView<ResourceID> progress) {
		SA_PROFILE_FUNCTION();

		if (!std::filesystem::exists(path)) {
			DEBUG_LOG_ERROR("No such file:", path);
			return NULL_RESOURCE;
		}
		std::filesystem::path absolutePath = std::filesystem::absolute(path);

		ResourceID id = ResourceManager::get().keyToID<ModelData>(absolutePath.string());
		if (id != NULL_RESOURCE) {
			return id;
		}

		id = ResourceManager::get().insert<ModelData>(absolutePath.string(), {});
		ModelData* model = ResourceManager::get().get<ModelData>(id);

		loadAssimpModel(path, model, progress);

		return id;
	}

	ProgressView<ResourceID> AssetManager::loadModelAsync(const std::filesystem::path& path) {
		SA_PROFILE_FUNCTION();
		ProgressView<ResourceID> p;
		auto future = m_taskExecutor.async([path, p]() {
			return AssetManager::get().loadModel(path, p);
		});
		p.setFuture(future.share());
		return p;
	}

	ResourceID AssetManager::loadDefaultMaterial() {
		ResourceID id = ResourceManager::get().keyToID<Material>("default_material");
		if (id != NULL_RESOURCE)
			return id;
		Material mat = {};
		memset(&mat.values, 0, sizeof(mat.values));
		mat.values.ambientColor = SA_COLOR_WHITE;
		mat.values.diffuseColor = SA_COLOR_WHITE;
		mat.values.specularColor = SA_COLOR_WHITE;
		mat.values.opacity = 1.0f;
		mat.values.shininess = 1.0f;
		mat.twoSided = false;

		return sa::ResourceManager::get().insert<Material>(mat);
	}

	ResourceID AssetManager::loadQuad() {
		ResourceManager& resManager = ResourceManager::get();
		Renderer& renderer = Renderer::get();

		ResourceID id = resManager.keyToID<ModelData>("Quad");
		if (id != NULL_RESOURCE) {
			return id;
		}

		
		Mesh mesh = {};
		
		std::vector<VertexUV> vertices = {
			{ glm::vec4(-0.5f, 0.5f, 0, 1), glm::vec2(0, 0) },
			{ glm::vec4(0.5f, 0.5f, 0, 1), glm::vec2(1, 0) },
			{ glm::vec4(0.5f, -0.5f, 0, 1), glm::vec2(1, 1) },
			{ glm::vec4(-0.5f, -0.5f, 0, 1), glm::vec2(0, 1) }
		};


		mesh.vertexBuffer = renderer.createBuffer(BufferType::VERTEX, vertices.size() * sizeof(VertexUV), vertices.data());

		std::vector<uint32_t> indices = {
			0, 1, 3,
			1, 2, 3
		};
		mesh.indexBuffer = renderer.createBuffer(BufferType::INDEX, indices.size() * sizeof(uint32_t), indices.data());

		ModelData model = {};
		model.meshes.push_back(mesh);
		
		id = resManager.insert<ModelData>("Quad", model);

		return id;
	}

	ModelData* AssetManager::getModel(ResourceID id) const {
		return ResourceManager::get().get<ModelData>(id);
	}

	Material* AssetManager::getMaterial(ResourceID id) const {
		return ResourceManager::get().get<Material>(id);
	}


	AssetManager::AssetManager()
		: m_nextID(0)
	{
		loadDefaultTexture();
	}

	void AssetManager::loadAssimpModel(const std::filesystem::path& path, ModelData* pModel, ProgressView<ResourceID> progress) {
		SA_PROFILE_FUNCTION();
		Assimp::Importer importer;

		unsigned int flags =
			aiProcessPreset_TargetRealtime_Quality |
			aiProcess_FlipUVs |
			aiProcess_PreTransformVertices |
			//aiProcess_MakeLeftHanded |
			0;
		if (!importer.ValidateFlags(flags)) {
			DEBUG_LOG_ERROR("Assimp Flag validation failed");
		}

		const aiScene* scene = importer.ReadFile(path.string(), flags);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			DEBUG_LOG_ERROR("Failed to read file ", path.string(), " : ", importer.GetErrorString());
			return;
		}

		DEBUG_LOG_INFO(
			"Loaded file", path.string(),
			"\nMeshes", scene->mNumMeshes,
			"\nAnimations", scene->mNumAnimations,
			"\nLights", scene->mNumLights);

		progress.setMaxCompletionCount(scene->mNumMeshes + scene->mNumMaterials);

		processNode(scene, scene->mRootNode, pModel, progress);

		// Materials

		DEBUG_LOG_INFO("Material Count:", scene->mNumMaterials);
		tf::Taskflow taskflow;
		

		
		

		std::vector<ResourceID> materials(scene->mNumMaterials);
		taskflow.for_each_index(0U, scene->mNumMaterials, 1U, [&](int i) {
				SA_PROFILE_SCOPE("Load material: " + std::to_string(i));
				aiMaterial* aMaterial = scene->mMaterials[i];
				Material material;

				// Diffuse Color
				material.values.diffuseColor = getColor(aMaterial, AI_MATKEY_COLOR_DIFFUSE);
				// Specular Color
				material.values.specularColor = getColor(aMaterial, AI_MATKEY_COLOR_SPECULAR);

				// Ambient Color
				material.values.ambientColor = getColor(aMaterial, AI_MATKEY_COLOR_AMBIENT);
				// Emissive Color
				material.values.emissiveColor = getColor(aMaterial, AI_MATKEY_COLOR_EMISSIVE, SA_COLOR_BLACK);

				aMaterial->Get(AI_MATKEY_OPACITY, material.values.opacity);
				aMaterial->Get(AI_MATKEY_SHININESS, material.values.shininess);
				aMaterial->Get(AI_MATKEY_METALLIC_FACTOR, material.values.metallic);
				aMaterial->Get(AI_MATKEY_TWOSIDED, material.twoSided);

				for (unsigned int j = aiTextureType::aiTextureType_NONE; j <= aiTextureType::aiTextureType_TRANSMISSION; j++) {
					loadMaterialTexture(material, path.parent_path(), (aiTextureType)j, scene->mTextures, aMaterial);
				}
				ResourceID matId = sa::ResourceManager::get().insert<Material>(material);
				materials[i] = matId;
			
				progress.increment();
			});
		//}

		m_taskExecutor.run_and_wait(taskflow);


		for (auto& mesh : pModel->meshes) {
			mesh.materialID = materials[mesh.materialID]; // swap index to material ID
		}
	}

	AssetManager::~AssetManager() {

	}
}