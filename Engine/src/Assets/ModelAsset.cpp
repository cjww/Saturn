#include "pch.h"
#include "ModelAsset.h"

namespace sa {
	bool ModelAsset::importFromFile(const std::filesystem::path& path) {

		m_id = AssetManager::get().loadModel(path);
		if (m_id == NULL_RESOURCE)
			return false;

		data = AssetManager::get().getModel(m_id);

		m_refCount = 1;
		m_isLoaded = true;

		m_header.type = AssetType::MODEL;
		m_header.offset = 0;
		m_header.size = 0;
		
		return true;
	}

	bool ModelAsset::loadFromPackage(std::ifstream& file) {
		readHeader(file);
		


		m_isLoaded = true;
		return true;
	}

	bool ModelAsset::loadFromFile(const std::filesystem::path& path) {
		std::ifstream file(path, std::ios::binary | std::ios::in);
		if (!file.good()) {
			file.close();
			return false;
		}
		
		readHeader(file);

		auto [id, modelData] = AssetManager::get().newModel(path.filename().generic_string());
		m_id = id;
		data = modelData;

		uint32_t meshCount = 0;
		file.read((char*)&meshCount, sizeof(meshCount));

		data->meshes.resize(meshCount);
		for (auto& mesh : data->meshes) {
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

	bool ModelAsset::writeToPackage(std::ofstream& file) {
		

		return true;
	}

	bool ModelAsset::writeToFile(const std::filesystem::path& path) {

		std::ofstream file(path, std::ios::binary | std::ios::out);
		if (!file.good()) {
			file.close();
			return false;
		}
		writeHeader(m_header, file);

		if (data) {
			uint32_t meshCount = data->meshes.size();
			file.write((char*)&meshCount, sizeof(meshCount));

			for (auto& mesh : data->meshes) {
				uint32_t vertexCount= mesh.vertices.size();
				file.write((char*)&vertexCount, sizeof(vertexCount));
				for (auto& vertex : mesh.vertices) {
					file.write((char*)&vertex, sizeof(vertex));
				}

				uint32_t indexCount= mesh.indices.size();
				file.write((char*)&indexCount, sizeof(indexCount));
				for (auto& index : mesh.indices) {
					file.write((char*)&index, sizeof(index));
				}
				file.write((char*)&mesh.materialID, sizeof(mesh.materialID));
			}
		}

		file.close();

		return true;
	}

	bool ModelAsset::load() {
		return loadFromFile(m_assetPath);
	}

	bool ModelAsset::write() {
		return writeToFile(m_assetPath);
	}

	ResourceID ModelAsset::getID() {
		return m_id;
	}



}