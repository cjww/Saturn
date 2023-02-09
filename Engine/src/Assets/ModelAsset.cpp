#include "pch.h"
#include "ModelAsset.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/color4.h"

#include "assimp/ProgressHandler.hpp"

#include "TextureAsset.h"
#include "MaterialAsset.h"

namespace sa {
	bool searchForFile(const std::filesystem::path& directory, const std::filesystem::path& filename, std::filesystem::path& outPath) {
		outPath = directory / filename;
		if (std::filesystem::exists(outPath)) {
			return true;
		}

		std::filesystem::recursive_directory_iterator it(directory);
		SA_DEBUG_LOG_INFO("Looking for file:", filename.filename());
		while (it != std::filesystem::end(it)) {

			if (it->path().filename() == filename.filename()) {
				outPath = it->path();
				SA_DEBUG_LOG_INFO("Found in:", outPath);
				return true;
			}
			it++;
		}
		std::filesystem::path dir = directory;
		if (!dir.has_parent_path()) {
			dir = std::filesystem::absolute(dir).parent_path();
		}
		std::filesystem::recursive_directory_iterator parentIt(dir);

		while (parentIt != std::filesystem::end(parentIt)) {
			if (parentIt->path() == directory) {
				parentIt.disable_recursion_pending();
			}
			else if (parentIt->path().filename() == filename.filename()) {
				outPath = parentIt->path();
				SA_DEBUG_LOG_INFO("Found in:", outPath);
				return true;
			}
			parentIt++;

		}

		return false;
	}

	void processNode(const aiScene* scene, const aiNode* node, ModelData* pModelData, ProgressView<bool>* pProgress) {
		SA_PROFILE_FUNCTION();
		for (int i = 0; i < node->mNumMeshes; i++) {
			Mesh mesh = {};
			const aiMesh* aMesh = scene->mMeshes[node->mMeshes[i]];

			std::vector<VertexNormalUV>& vertices = mesh.vertices;
			std::vector<uint32_t>& indices = mesh.indices;

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
			mesh.materialID = aMesh->mMaterialIndex;
			pModelData->meshes.push_back(mesh);
			pProgress->increment();
		}

		for (int i = 0; i < node->mNumChildren; i++) {
			processNode(scene, node->mChildren[i], pModelData, pProgress);
		}
	}

	void loadMaterialTexture(Material& material, const std::filesystem::path& directory, aiTextureType type, aiTexture** ppAiTextures, aiMaterial* pAiMaterial, const std::filesystem::path& textureDir) {
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
					filename = std::string(pAiTex->mFilename.data) + "." + std::string(pAiTex->achFormatHint);
				}
				else { // uncompressed
					filename = std::string(pAiTex->mFilename.data); // read from file anyway
				}
			}
			std::filesystem::path finalPath;
			if (!searchForFile(directory, filename, finalPath)) {
				SA_DEBUG_LOG_ERROR("File not found: ", filename);
				continue;
			}

			TextureAsset* tex = AssetManager::get().importAsset<TextureAsset>(finalPath, textureDir);
			if (!tex) {
				SA_DEBUG_LOG_ERROR("Failed to create texture, ", finalPath.generic_string());
				continue;
			}

			textures[i].textureAssetID = tex->getHeader().id;
			textures[i].blendFactor = blending;
			textures[i].blendOp = (TextureBlendOp)op;

			SA_DEBUG_LOG_INFO("Loaded texture: ", finalPath.filename(), ", type:", toString((MaterialTextureType)type), ", Material: ", &pAiMaterial);
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

	bool ModelAsset::loadAssimpModel(const std::filesystem::path& path) {
		SA_PROFILE_FUNCTION();
		Assimp::Importer importer;

		unsigned int flags =
			aiProcessPreset_TargetRealtime_Quality |
			aiProcess_FlipUVs |
			aiProcess_PreTransformVertices |
			//aiProcess_MakeLeftHanded |
			0;
		if (!importer.ValidateFlags(flags)) {
			SA_DEBUG_LOG_ERROR("Assimp Flag validation failed");
			return false;
		}

		const aiScene* scene = importer.ReadFile(path.generic_string(), flags);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			SA_DEBUG_LOG_ERROR("Failed to read file ", path.generic_string(), " : ", importer.GetErrorString());
			return false;
		}

		SA_DEBUG_LOG_INFO(
			"Loaded file", path.generic_string(),
			"\nMeshes", scene->mNumMeshes,
			"\nAnimations", scene->mNumAnimations,
			"\nLights", scene->mNumLights);

		m_progress.setMaxCompletionCount(scene->mNumMeshes);

		processNode(scene, scene->mRootNode, &data, &m_progress);

		// Materials
		SA_DEBUG_LOG_INFO("Material Count:", scene->mNumMaterials);
		tf::Taskflow taskflow;


		std::vector<MaterialAsset*> materials(scene->mNumMaterials);
		if (scene->mNumMaterials > 0 || scene->mNumTextures > 0) {
			auto filename = m_assetPath.filename();
			auto dirname = m_assetPath.parent_path() / filename;
			dirname.replace_extension();
			
			int i = 1;
			while (std::filesystem::exists(dirname)) {
				dirname.replace_filename(dirname.filename().generic_string() + std::to_string(i));
				i++;
			}
			std::filesystem::create_directory(dirname);
			m_assetPath = dirname / filename;
		}

		auto materialDir = m_assetPath.parent_path() / "Materials";
		if (!std::filesystem::exists(materialDir)) {
			std::filesystem::create_directory(materialDir);
		}

		auto textureDir = m_assetPath.parent_path() / "Textures";
		if (!std::filesystem::exists(textureDir)) {
			std::filesystem::create_directory(textureDir);
		}

		taskflow.for_each_index(0U, scene->mNumMaterials, 1U, [&](int i) {
			aiMaterial* aMaterial = scene->mMaterials[i];
			SA_PROFILE_SCOPE(path.generic_string() + ", Load material [" + std::to_string(i) + "] " + aMaterial->GetName().C_Str());
			SA_DEBUG_LOG_INFO("Load material: ", path.generic_string(), "-", aMaterial->GetName().C_Str());

			MaterialAsset* materialAsset = AssetManager::get().createAsset<MaterialAsset>(aMaterial->GetName().C_Str(), materialDir);
			Material& material = materialAsset->data;
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
				loadMaterialTexture(material, path.parent_path(), (aiTextureType)j, scene->mTextures, aMaterial, textureDir);
			}
			material.update();
			materials[i] = materialAsset;
		});

		m_taskExecutor.run_and_wait(taskflow);


		for (auto& mesh : data.meshes) {
			mesh.materialID = materials[mesh.materialID]->getHeader().id; // swap index to material ID
		}
	}

	bool ModelAsset::importFromFile(const std::filesystem::path& path) {
		if (!std::filesystem::exists(path)) {
			return false;
		}

		m_refCount = 1;
		m_header.type = AssetType::MODEL;
		m_header.offset = 0;
		m_header.size = 0;
		
		m_progress.reset();

		auto future = m_taskExecutor.async([&, path]() {
			if (loadAssimpModel(path)) {
				m_isLoaded = true;
				return true;
			}
			return false;
		});

		m_progress.setFuture(future.share());
		
		return true;
	}

	bool ModelAsset::load() {

		std::ifstream file(m_assetPath, std::ios::binary | std::ios::in);
		if (!file.good()) {
			file.close();
			return false;
		}

		readHeader(file);

		/*
		auto [id, modelData] = AssetManager::get().newModel(path.filename().generic_string());
		m_id = id;
		data = modelData;
		*/

		uint32_t meshCount = 0;
		file.read((char*)&meshCount, sizeof(meshCount));

		data.meshes.resize(meshCount);
		for (auto& mesh : data.meshes) {
			uint32_t vertexCount = 0;
			file.read((char*)&vertexCount, sizeof(vertexCount));

			mesh.vertices.resize(vertexCount);
			for (auto& vertex : mesh.vertices) {
				file.read((char*)&vertex, sizeof(vertex));
			}

			uint32_t indexCount = 0;
			file.read((char*)&indexCount, sizeof(indexCount));

			mesh.indices.resize(indexCount);
			for (auto& index : mesh.indices) {
				file.read((char*)&index, sizeof(index));
			}
			file.read((char*)&mesh.materialID, sizeof(mesh.materialID));
		}

		file.close();
		m_isLoaded = true;
		return true;
	}

	bool ModelAsset::write() {
		if (!m_isLoaded)
			return false;

		std::ofstream file(m_assetPath, std::ios::binary | std::ios::out);
		if (!file.good()) {
			file.close();
			return false;
		}
		writeHeader(m_header, file);


		uint32_t meshCount = data.meshes.size();
		file.write((char*)&meshCount, sizeof(meshCount));

		for (auto& mesh : data.meshes) {
			uint32_t vertexCount = mesh.vertices.size();
			file.write((char*)&vertexCount, sizeof(vertexCount));
			for (auto& vertex : mesh.vertices) {
				file.write((char*)&vertex, sizeof(vertex));
			}

			uint32_t indexCount = mesh.indices.size();
			file.write((char*)&indexCount, sizeof(indexCount));
			for (auto& index : mesh.indices) {
				file.write((char*)&index, sizeof(index));
			}
			file.write((char*)&mesh.materialID, sizeof(mesh.materialID));
		}


		file.close();

		return true;
	}

	ResourceID ModelAsset::getID() {
		return m_id;
	}



}