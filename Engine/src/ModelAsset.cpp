#include "pch.h"
#include "Assets/ModelAsset.h"
#include "Assets/TextureAsset.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/color4.h"

#include "assimp/ProgressHandler.hpp"


#include "AssetManager.h"

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

	void ModelAsset::processNode(const void* pScene, const void* pNode, std::vector<uint32_t>& materialIndices) {
		SA_PROFILE_FUNCTION();
		const aiScene* scene = (const aiScene*)pScene;
		const aiNode* node = (const aiNode*)pNode;

		SA_DEBUG_LOG_INFO("> ", node->mName.C_Str(), 
			" { Parent: ", (node->mParent ? node->mParent->mName.C_Str() : "None"), 
			", Meshes: ", node->mNumMeshes);

		for (int i = 0; i < node->mNumMeshes; i++) {
			Mesh mesh = {};
			const aiMesh* aMesh = scene->mMeshes[node->mMeshes[i]];
			SA_DEBUG_LOG_INFO("Mesh: ", aMesh->mName.C_Str());
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
			materialIndices.push_back(aMesh->mMaterialIndex);
			data.meshes.emplace_back(std::move(mesh));
			incrementProgress();
		}

		for (int i = 0; i < node->mNumChildren; i++) {
			processNode(scene, node->mChildren[i], materialIndices);
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

			TextureAsset* tex = AssetManager::Get().importAsset<TextureAsset>(finalPath, textureDir);
			if (!tex) {
				SA_DEBUG_LOG_ERROR("Failed to create texture, ", finalPath.generic_string());
				continue;
			}

			textures[i].textureAssetID = tex->getHeader().id;
			textures[i].blendFactor = blending;
			textures[i].blendOp = (TextureBlendOp)op;

			SA_DEBUG_LOG_INFO("Loaded texture: ", finalPath.filename(), ", type:", sa::to_string((MaterialTextureType)type), ", Material: ", &pAiMaterial);
		}

		material.setTextures(textures, (MaterialTextureType)type);
	}

	sa::Color getColor(aiMaterial* pMaterial, const char* pKey, unsigned int type, unsigned int idx, sa::Color defaultColor = sa::Color::White) {
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

		setCompletionCount(scene->mNumMeshes + scene->mNumMaterials);
		std::vector<uint32_t> materialIndices;
		materialIndices.reserve(scene->mNumMeshes);
		processNode(scene, scene->mRootNode, materialIndices);

		// Materials
		SA_DEBUG_LOG_INFO("Material Count:", scene->mNumMaterials);


		std::vector<Material*> materials(scene->mNumMaterials);
		if (scene->mNumMaterials > 0 || scene->mNumTextures > 0) {
			auto filename = getAssetPath().filename();
			auto dirname = getAssetPath().parent_path() / filename;
			dirname.replace_extension();

			std::filesystem::create_directory(dirname);
			setAssetPath(dirname / filename);
		}

		auto materialDir = getAssetPath().parent_path() / "Materials";
		std::filesystem::create_directory(materialDir);
		

		auto textureDir = getAssetPath().parent_path() / "Textures";
		std::filesystem::create_directory(textureDir);
		
		
		tf::Taskflow taskflow;
		taskflow.for_each_index(0U, scene->mNumMaterials, 1U, [&](int i) {
		//for (uint32_t i = 0; i < scene->mNumMaterials; i++) {
			aiMaterial* aMaterial = scene->mMaterials[i];
			SA_PROFILE_SCOPE(path.generic_string() + ", Load material [" + std::to_string(i) + "] " + aMaterial->GetName().C_Str());
			SA_DEBUG_LOG_INFO("Load material: ", path.generic_string(), "-", aMaterial->GetName().C_Str());

			Material* pMaterial = AssetManager::Get().findAssetByPath<Material>(materialDir / aMaterial->GetName().C_Str());
			if(!pMaterial)
				pMaterial = AssetManager::Get().createAsset<Material>(aMaterial->GetName().C_Str(), materialDir);

			// Base Color
			//pMaterial->values.albedoColor = getColor(aMaterial, AI_MATKEY_COLOR_DIFFUSE);
			pMaterial->values.albedoColor = getColor(aMaterial, AI_MATKEY_BASE_COLOR);
			
			// Emissive Color
			pMaterial->values.emissiveColor = getColor(aMaterial, AI_MATKEY_COLOR_EMISSIVE, Color::Black);

			aMaterial->Get(AI_MATKEY_EMISSIVE_INTENSITY, pMaterial->values.emissiveColor.a);
			aMaterial->Get(AI_MATKEY_OPACITY, pMaterial->values.opacity);
			aMaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR, pMaterial->values.roughness);
			aMaterial->Get(AI_MATKEY_METALLIC_FACTOR, pMaterial->values.metallic);
			aMaterial->Get(AI_MATKEY_TWOSIDED, pMaterial->twoSided);

			// PBR textures
			loadMaterialTexture(*pMaterial, path.parent_path(), aiTextureType_BASE_COLOR, scene->mTextures, aMaterial, textureDir);
			loadMaterialTexture(*pMaterial, path.parent_path(), aiTextureType_NORMAL_CAMERA, scene->mTextures, aMaterial, textureDir);
			loadMaterialTexture(*pMaterial, path.parent_path(), aiTextureType_EMISSION_COLOR, scene->mTextures, aMaterial, textureDir);
			loadMaterialTexture(*pMaterial, path.parent_path(), aiTextureType_METALNESS, scene->mTextures, aMaterial, textureDir);
			loadMaterialTexture(*pMaterial, path.parent_path(), aiTextureType_DIFFUSE_ROUGHNESS, scene->mTextures, aMaterial, textureDir);
			loadMaterialTexture(*pMaterial, path.parent_path(), aiTextureType_AMBIENT_OCCLUSION, scene->mTextures, aMaterial, textureDir);

			pMaterial->update();
			materials[i] = pMaterial;
			pMaterial->write();
			incrementProgress();
		//}
		});

		runTaskflow(taskflow).wait();

		int i = 0;
		for (auto& mesh : data.meshes) {
			mesh.material = materials[materialIndices[i]]; // swap index to material ID
			++i;
		}
		return true;
	}

	bool ModelAsset::IsExtensionSupported(const std::string& extension) {
		Assimp::Importer importer;
		return importer.IsExtensionSupported(extension);
	}

	bool ModelAsset::onImport(const std::filesystem::path& path) {
		sa::Clock clock;
		bool sucess = loadAssimpModel(path);
		SA_DEBUG_LOG_INFO("Finished importing ", path, ": ", clock.getElapsedTime<std::chrono::milliseconds>(), "ms");
		return sucess;
	}

	bool ModelAsset::onLoad(std::ifstream& file, AssetLoadFlags flags) {
		uint32_t meshCount = 0;
		file.read(reinterpret_cast<char*>(&meshCount), sizeof(meshCount));

		data.meshes.resize(meshCount);
		for (auto& mesh : data.meshes) {
			uint32_t vertexCount = 0;
			file.read(reinterpret_cast<char*>(&vertexCount), sizeof(vertexCount));
			mesh.vertices.resize(vertexCount);
			file.read(reinterpret_cast<char*>(mesh.vertices.data()), sizeof(sa::VertexNormalUV) * vertexCount);

			uint32_t indexCount = 0;
			file.read(reinterpret_cast<char*>(&indexCount), sizeof(indexCount));
			mesh.indices.resize(indexCount);
			file.read(reinterpret_cast<char*>(mesh.indices.data()), sizeof(uint32_t) * indexCount);
			UUID materialID = SA_DEFAULT_MATERIAL_ID;
			file.read(reinterpret_cast<char*>(&materialID), sizeof(materialID));
			mesh.material = materialID;

			if(const auto pProgress = mesh.material.getProgress())
				addDependency(*pProgress);
			
		}
		return true;
	}

	bool ModelAsset::onWrite(std::ofstream& file, AssetWriteFlags flags) {
		uint32_t meshCount = data.meshes.size();
		file.write(reinterpret_cast<char*>(&meshCount), sizeof(meshCount));

		for (auto& mesh : data.meshes) {
			uint32_t vertexCount = mesh.vertices.size();
			file.write(reinterpret_cast<char*>(&vertexCount), sizeof(vertexCount));
			if (vertexCount > 0)
				file.write(reinterpret_cast<char*>(mesh.vertices.data()), sizeof(sa::VertexNormalUV) * vertexCount);
			
			uint32_t indexCount = mesh.indices.size();
			file.write(reinterpret_cast<char*>(&indexCount), sizeof(indexCount));
			
			if (indexCount > 0)
				file.write(reinterpret_cast<char*>(mesh.indices.data()), sizeof(uint32_t) * indexCount);
			
			UUID materialID = mesh.material ? mesh.material.getID() : static_cast<UUID>(SA_DEFAULT_MATERIAL_ID);
			file.write(reinterpret_cast<const char*>(&materialID), sizeof(materialID));
		}

		return true;
	}

	bool ModelAsset::onUnload() {
		data.meshes.clear();
		data.meshes.shrink_to_fit();
		return true;
	}
}