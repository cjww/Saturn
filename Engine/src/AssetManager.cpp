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

	std::vector<std::filesystem::path> g_texturePaths = {
		"",
		"textures",
		"Textures",
		"texture",
		"Texture",
		"../Texture",
		"../texture",
		"../Textures",
		"../textures",
	};

	void processNode(const aiScene* scene, const aiNode* node, ModelData* pModelData) {
		for (int i = 0; i < node->mNumMeshes; i++) {
			Mesh mesh = {};
			const aiMesh* aMesh = scene->mMeshes[node->mMeshes[i]];
			
			std::vector<VertexUV> vertices;
			std::vector<uint32_t> indices;

			std::unordered_map<VertexUV, uint32_t> vertexIndices;
			
			for (int j = 0; j < aMesh->mNumFaces; j++) {
				aiFace face = aMesh->mFaces[j];

				for (int k = 0; k < face.mNumIndices; k++) {
					sa::VertexUV vertex = {};
					
					aiVector3D pos = aMesh->mVertices[face.mIndices[k]];
					pos = node->mTransformation * pos; // TODO not sure how good this is

					vertex.position = { pos.x, pos.y, pos.z , 1.f };
					aiVector3D texCoord = aMesh->mTextureCoords[0][face.mIndices[k]]; // assumes 1 tex coord per vertex
					vertex.texCoord = { texCoord.x, texCoord.y };
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
		}

		for (int i = 0; i < node->mNumChildren; i++) {
			processNode(scene, node->mChildren[i], pModelData);
		}
	}

	std::vector<Texture2D> loadMaterialTexture(const std::filesystem::path& directory, aiMaterial* pMaterial, aiTextureType type, std::string typeName) {
		std::vector<Texture2D> textures;
		for (unsigned int i = 0; i < pMaterial->GetTextureCount(type); i++) {
			aiString str;
			pMaterial->GetTexture(type, i, &str);
			std::filesystem::path path(str.C_Str());
			
			for (auto it = g_texturePaths.begin(); it != g_texturePaths.end() && !std::filesystem::exists(path); it++) {
				path = directory / *it / path.filename();
				DEBUG_LOG_INFO("Looking for file in", path);
			}
			if (!std::filesystem::exists(path)) {
				DEBUG_LOG_ERROR("File not found:", path.filename());
				continue;
			}
			
			path = std::filesystem::absolute(path);


			Texture2D* tex = AssetManager::get().loadTexture(path, true);
			if (!tex) {
				DEBUG_LOG_ERROR("Failed to create texture,", path.string());
				continue;
			}
			textures.push_back(*tex);
			
		}
		return textures;
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

	ResourceID AssetManager::loadModel(const std::filesystem::path& path) {

		if (!std::filesystem::exists(path)) {
			return NULL_RESOURCE;
		}

		ResourceID id = ResourceManager::get().keyToID<ModelData>(path.string());
		if (id != NULL_RESOURCE) {
			return id;
		}

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path.string(),
			aiProcess_Triangulate |
			aiProcess_FlipUVs |
			aiProcess_JoinIdenticalVertices |
			0
		);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			DEBUG_LOG_ERROR("Failed to read file ", path.string(), " : ", importer.GetErrorString());
		}

		DEBUG_LOG_INFO("Loaded file", path.string());
		DEBUG_LOG_INFO("Meshes", scene->mNumMeshes);
		for (int i = 0; i < scene->mNumMeshes; i++) {
			aiMesh* m = scene->mMeshes[i];
			DEBUG_LOG_INFO("\t", i);
			DEBUG_LOG_INFO("\tVertices", m->mNumVertices);
			DEBUG_LOG_INFO("\tBones", m->mNumBones);
			DEBUG_LOG_INFO("\tMaterial", m->mMaterialIndex);
		}

		DEBUG_LOG_INFO("Animations", scene->mNumAnimations);
		DEBUG_LOG_INFO("Lights", scene->mNumLights);



		id = ResourceManager::get().insert<ModelData>(path.string(), {});
		ModelData* model = ResourceManager::get().get<ModelData>(id);

		processNode(scene, scene->mRootNode, model);

		std::vector<ResourceID> materials;
		
		// Materials
		DEBUG_LOG_INFO("Material Count:", scene->mNumMaterials);
		for (int i = 0; i < scene->mNumMaterials; i++) {
			aiMaterial* aMaterial = scene->mMaterials[i];
			Material material;

			DEBUG_LOG_INFO("\t[", i, "]:");

			// Diffuse Color
			aiColor3D color = {};
			aMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color);
			if(!color.IsBlack())
				material.values.diffuseColor = { color.r, color.g, color.b, 1 };
			
			DEBUG_LOG_INFO("\tDiffuse color {", material.values.diffuseColor.r, material.values.diffuseColor.g, material.values.diffuseColor.b, "}");
			
			// Diffuse Texture
			std::vector<Texture2D> diffuseTextures = loadMaterialTexture(path.parent_path(), aMaterial, aiTextureType::aiTextureType_DIFFUSE, "texture_diffuse");
			if (!diffuseTextures.empty()) {
				material.setDiffuseMaps(diffuseTextures);
				DEBUG_LOG_INFO("\tDiffuse Map found");
			}

			// Base Texture
			std::vector<Texture2D> baseColorTextures = loadMaterialTexture(path.parent_path(), aMaterial, aiTextureType::aiTextureType_BASE_COLOR, "texture_base");
			if (!baseColorTextures.empty()) {
				material.setDiffuseMaps(baseColorTextures);
				DEBUG_LOG_INFO("\Base Map found");
			}


			ResourceID matId = sa::ResourceManager::get().insert<Material>(material);
			DEBUG_LOG_INFO("\tRecieved ID:", matId);
			materials.push_back(matId);
		}

		for (auto& mesh : model->meshes) {
			mesh.materialID = materials[mesh.materialID]; // swap index to material ID
		}

		return id;
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

	AssetManager::~AssetManager() {

	}
}