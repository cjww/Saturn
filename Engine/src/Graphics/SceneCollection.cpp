#include "pch.h"
#include "SceneCollection.h"

namespace sa {
	SceneCollection::SceneCollection() {
		m_objectCount = 0;
		m_vertexCount = 0;
		m_indexCount = 0;
		m_uniqueMeshCount = 0;

		sa::Renderer& renderer = sa::Renderer::get();

		uint32_t lightCount = 0U;
		m_lightBuffer = renderer.createDynamicBuffer(BufferType::STORAGE, sizeof(uint32_t), &lightCount);

		m_indirectIndexedBuffer = renderer.createDynamicBuffer(BufferType::INDIRECT);
		m_vertexBuffer = renderer.createDynamicBuffer(BufferType::VERTEX);
		m_indexBuffer = renderer.createDynamicBuffer(BufferType::INDEX);
		m_objectBuffer = renderer.createDynamicBuffer(BufferType::STORAGE);

		m_materialBuffer = renderer.createDynamicBuffer(BufferType::STORAGE);
		m_materialIndicesBuffer = renderer.createDynamicBuffer(BufferType::STORAGE);

	}

	void SceneCollection::clear() {
		m_models.clear();
		m_objects.clear();
		m_textures.clear();
		m_materials.clear();
		m_materialData.clear();
		m_materialIndices.clear();
		m_lights.clear();

		m_objectCount = 0;
		m_vertexCount = 0;
		m_indexCount = 0;
		m_uniqueMeshCount = 0;
	}

	void SceneCollection::addObject(glm::mat4 transformation, ModelAsset* pModelAsset) {
		SA_PROFILE_FUNCTION();

		if (!pModelAsset || !pModelAsset->isLoaded())
			return;
		ModelData* pModel = &pModelAsset->data;

		ObjectData objectBuffer = {};
		objectBuffer.worldMat = transformation;

		auto it = std::find(m_models.begin(), m_models.end(), pModelAsset);
		if (it == m_models.end()) {
			m_models.push_back(pModelAsset);
			if (m_objects.size() >= m_models.size()) {
				m_objects[m_models.size() - 1].clear();
				m_objects[m_models.size() - 1].push_back(objectBuffer);
			}
			else {
				m_objects.push_back({ objectBuffer });
			}
			for (const auto& mesh : pModel->meshes) {
				m_vertexCount += mesh.vertices.size();
				m_indexCount += mesh.indices.size();
				m_uniqueMeshCount++;
			}
		}
		else {
			m_objects[std::distance(m_models.begin(), it)].push_back(objectBuffer);
		}
		m_objectCount++;
	}

	void SceneCollection::addLight(const LightData& light) {
		m_lights.push_back(light);
	}

	void SceneCollection::makeRenderReady() {
		SA_PROFILE_FUNCTION();

	

		//Ligths
		m_lightBuffer.write(static_cast<uint32_t>(m_lights.size()));
		m_lightBuffer.append(m_lights, 16);


		// Clear Dynamic buffers
		m_objectBuffer.clear();
		m_indirectIndexedBuffer.clear();
		m_vertexBuffer.clear();
		m_indexBuffer.clear();
		m_materialBuffer.clear();
		m_materialIndicesBuffer.clear();

		// reserve dynamic buffers
		m_objectBuffer.reserve(m_objectCount * sizeof(ObjectData), IGNORE_CONTENT);
		m_indirectIndexedBuffer.reserve(m_uniqueMeshCount * sizeof(DrawIndexedIndirectCommand), IGNORE_CONTENT);
		m_vertexBuffer.reserve(m_vertexCount * sizeof(VertexNormalUV), IGNORE_CONTENT);
		m_indexBuffer.reserve(m_indexCount * sizeof(uint32_t), IGNORE_CONTENT);
		m_materialBuffer.reserve(m_uniqueMeshCount * sizeof(Material::Values), IGNORE_CONTENT);
		m_materialIndicesBuffer.reserve(m_uniqueMeshCount * sizeof(int32_t), IGNORE_CONTENT);

		uint32_t firstInstance = 0;

		int32_t materialCount = 0;
		uint32_t meshCount = 0;

		for (size_t i = 0; i < m_models.size(); i++) {
			for (const auto& objectBuffer : m_objects[i]) {
				m_objectBuffer << objectBuffer;
			}
			ModelData* pModel = &m_models[i]->data;

			for (const auto& mesh : pModel->meshes) {

				// Push mesh into buffers
				uint32_t vertexOffset = m_vertexBuffer.getElementCount<VertexNormalUV>();
				m_vertexBuffer << mesh.vertices;

				uint32_t firstIndex = m_indexBuffer.getElementCount<uint32_t>();
				m_indexBuffer << mesh.indices;

				// Create a draw command for this mesh
				DrawIndexedIndirectCommand cmd = {};
				cmd.firstIndex = firstIndex;
				cmd.indexCount = mesh.indices.size();
				cmd.firstInstance = firstInstance;
				cmd.instanceCount = m_objects[i].size();
				cmd.vertexOffset = vertexOffset;
				m_indirectIndexedBuffer << cmd;

				//Material
				Material* pMaterial = AssetManager::get().getAsset<Material>(mesh.materialID);
				if (pMaterial && pMaterial->isLoaded()) {
					auto it = std::find(m_materials.begin(), m_materials.end(), pMaterial);
					if (it == m_materials.end()) {
						uint32_t textureOffset = m_textures.size();
						const std::vector<Texture>& matTextures = pMaterial->fetchTextures();
						m_textures.insert(m_textures.end(), matTextures.begin(), matTextures.end());

						Material::Values values = pMaterial->values;
						values.diffuseMapFirst = values.diffuseMapFirst + textureOffset;
						values.emissiveMapFirst = values.emissiveMapFirst + textureOffset;
						values.lightMapFirst = values.lightMapFirst + textureOffset;
						values.normalMapFirst = values.normalMapFirst + textureOffset;
						values.specularMapFirst = values.specularMapFirst + textureOffset;

						m_materials.push_back(pMaterial);
						m_materialData.push_back(values);
						m_materialIndices.push_back(materialCount);
						materialCount++;
					}
					else {
						m_materialIndices.push_back(std::distance(m_materials.begin(), it));
					}
				}
				else {
					m_materialIndices.push_back(-1); // Default Material in shader
				}
				meshCount++;
			}
			firstInstance += m_objects[i].size();
		}

		m_materialBuffer.write(m_materialData);
		m_materialIndicesBuffer.write(m_materialIndices);
	}

	void SceneCollection::swap() {
		m_lightBuffer.swap();
		m_vertexBuffer.swap();
		m_indexBuffer.swap();
		m_indirectIndexedBuffer.swap();
		m_objectBuffer.swap();
		m_materialBuffer.swap();
		m_materialIndicesBuffer.swap();
	}
	
	const Buffer& SceneCollection::getLightBuffer() const {
		return m_lightBuffer.getBuffer();
	}

	const Buffer& SceneCollection::getVertexBuffer() const {
		return m_vertexBuffer.getBuffer();
	}

	const Buffer& SceneCollection::getIndexBuffer() const {
		return m_indexBuffer.getBuffer();
	}

	const Buffer& SceneCollection::getDrawCommandBuffer() const {
		return m_indirectIndexedBuffer.getBuffer();
	}
	
	const Buffer& SceneCollection::getObjectBuffer() const {
		return m_objectBuffer.getBuffer();
	}
	
	const Buffer& SceneCollection::getMaterialBuffer() const {
		return m_materialBuffer.getBuffer();
	}
	
	const Buffer& SceneCollection::getMaterialIndicesBuffer() const {
		return m_materialIndicesBuffer.getBuffer();
	}

	const std::vector<Texture>& SceneCollection::getTextures() const {
		return m_textures;
	}
}