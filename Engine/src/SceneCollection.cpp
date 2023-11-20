#include "pch.h"
#include "Graphics/SceneCollection.h"

#include "Scene.h"

namespace sa {
	MaterialShaderCollection::MaterialShaderCollection(MaterialShader* pMaterialShader) {
		m_pMaterialShader = pMaterialShader;
		m_objectCount = 0;
		m_vertexCount = 0;
		m_indexCount = 0;
		m_uniqueMeshCount = 0;

		sa::Renderer& renderer = sa::Renderer::get();

		m_indirectIndexedBuffer = renderer.createDynamicBuffer(BufferType::INDIRECT);
		m_vertexBuffer = renderer.createDynamicBuffer(BufferType::VERTEX);
		m_indexBuffer = renderer.createDynamicBuffer(BufferType::INDEX);
		m_objectBuffer = renderer.createDynamicBuffer(BufferType::STORAGE);

		m_materialBuffer = renderer.createDynamicBuffer(BufferType::STORAGE);
		m_materialIndicesBuffer = renderer.createDynamicBuffer(BufferType::STORAGE);

		m_currentExtent = { 0, 0 };
	}

	void MaterialShaderCollection::addMesh(ModelAsset* pModelAsset, uint32_t meshIndex, const ObjectData& objectData) {

		auto it = std::find(m_models.begin(), m_models.end(), pModelAsset);
		size_t modelIndex = std::distance(m_models.begin(), it);
		bool uniqueModel = it == m_models.end();
		if (uniqueModel) {
			modelIndex = m_models.size();
			m_models.emplace_back(pModelAsset);
			m_meshes.push_back({});
			if (m_objects.size() >= m_models.size()) {
				m_objects[modelIndex].clear();
				m_objects[modelIndex].push_back(objectData);
			}
			else {
				m_objects.push_back({ objectData });
			}
		}
		else {
			m_objects[modelIndex].push_back(objectData);
		}
		m_objectCount++;
			
		{
			auto it = std::find(m_meshes[modelIndex].begin(), m_meshes[modelIndex].end(), meshIndex);
			if(it == m_meshes[modelIndex].end()) {
				m_meshes[modelIndex].push_back(meshIndex);
				const Mesh& mesh = pModelAsset->data.meshes[meshIndex];
				m_vertexCount += mesh.vertices.size();
				m_indexCount += mesh.indices.size();
				m_uniqueMeshCount++;
			}
		}
		
	}

	void MaterialShaderCollection::clear() {
		m_models.clear();
		m_meshes.clear();  // frees memory
		m_objects.clear(); // frees memory
		m_textures.clear();
		m_materials.clear();
		m_materialData.clear();
		m_materialIndices.clear();
		
		m_objectCount = 0;
		m_vertexCount = 0;
		m_indexCount = 0;
		m_uniqueMeshCount = 0;
	}

	void MaterialShaderCollection::swap() {
		m_vertexBuffer.swap();
		m_indexBuffer.swap();
		m_indirectIndexedBuffer.swap();
		m_objectBuffer.swap();
		m_materialBuffer.swap();
		m_materialIndicesBuffer.swap();
	}

	void MaterialShaderCollection::readyDescriptorSets() {
		if (!m_pMaterialShader->isLoaded())
			return;

		if (m_sceneDescriptorSetColorPass == NULL_RESOURCE || !m_pMaterialShader->m_colorShaderSet.hasAllocatedDescriptorSet(m_sceneDescriptorSetColorPass)) {
			m_sceneDescriptorSetColorPass = m_pMaterialShader->m_colorShaderSet.allocateDescriptorSet(SET_PER_FRAME);
		}

		if (m_sceneDescriptorSetDepthPass == NULL_RESOURCE || !m_pMaterialShader->m_depthShaderSet.hasAllocatedDescriptorSet(m_sceneDescriptorSetDepthPass)) {
			m_sceneDescriptorSetDepthPass = m_pMaterialShader->m_depthShaderSet.allocateDescriptorSet(SET_PER_FRAME);
		}
	}


	void MaterialShaderCollection::recreatePipelines(ResourceID colorRenderProgram, ResourceID depthRenderProgram, Extent extent) {
		if (extent == m_currentExtent && !m_pMaterialShader->m_recompiled)
			return;

		PipelineSettings settings = {};
		settings.dynamicStates.push_back(sa::VIEWPORT);

		auto& renderer = Renderer::get();
		if (m_colorPipeline != NULL_RESOURCE) {
			renderer.destroyPipeline(m_colorPipeline);
		}
		if (m_depthPipeline != NULL_RESOURCE) {
			renderer.destroyPipeline(m_depthPipeline);
		}

		m_colorPipeline = renderer.createGraphicsPipeline(colorRenderProgram, 0, extent, m_pMaterialShader->m_colorShaderSet, settings);
		m_depthPipeline = renderer.createGraphicsPipeline(depthRenderProgram, 0, extent, m_pMaterialShader->m_depthShaderSet, settings);
		m_currentExtent = extent;
		m_pMaterialShader->m_recompiled = false;
	}

	void MaterialShaderCollection::bindColorPipeline(RenderContext& context) {
		context.bindPipeline(m_colorPipeline);
	}

	void MaterialShaderCollection::bindDepthPipeline(RenderContext& context) {
		context.bindPipeline(m_depthPipeline);
	}

	
	MaterialShaderCollection& SceneCollection::getMaterialShaderCollection(MaterialShader* pMaterialShader) {
		if (!pMaterialShader)
			pMaterialShader = AssetManager::get().getDefaultMaterialShader();

		const auto it = std::find_if(m_materialShaderCollections.begin(), m_materialShaderCollections.end(), 
			[&](const MaterialShaderCollection& collection) { return collection.getMaterialShader() == pMaterialShader; });
		if (it == m_materialShaderCollections.end()) {
			return m_materialShaderCollections.emplace_back(pMaterialShader);
		}
		return *it;
	}

	const Buffer& MaterialShaderCollection::getVertexBuffer() const {
		return m_vertexBuffer.getBuffer();
	}

	const Buffer& MaterialShaderCollection::getIndexBuffer() const {
		return m_indexBuffer.getBuffer();
	}

	const Buffer& MaterialShaderCollection::getDrawCommandBuffer() const {
		return m_indirectIndexedBuffer.getBuffer();
	}

	const Buffer& MaterialShaderCollection::getObjectBuffer() const {
		return m_objectBuffer.getBuffer();
	}

	const Buffer& MaterialShaderCollection::getMaterialBuffer() const {
		return m_materialBuffer.getBuffer();
	}

	const Buffer& MaterialShaderCollection::getMaterialIndicesBuffer() const {
		return m_materialIndicesBuffer.getBuffer();
	}

	const std::vector<Texture>& MaterialShaderCollection::getTextures() const {
		return m_textures;
	}

	ResourceID MaterialShaderCollection::getSceneDescriptorSetColorPass() const {
		return m_sceneDescriptorSetColorPass;
	}

	ResourceID MaterialShaderCollection::getSceneDescriptorSetDepthPass() const {
		return m_sceneDescriptorSetDepthPass;
	}

	MaterialShader* MaterialShaderCollection::getMaterialShader() const {
		return m_pMaterialShader;
	}

	SceneCollection::SceneCollection() {
		sa::Renderer& renderer = sa::Renderer::get();

		uint32_t lightCount = 0U;
		m_lightBuffer = renderer.createDynamicBuffer(BufferType::STORAGE, sizeof(uint32_t), &lightCount);

		SA_DEBUG_LOG_INFO("SceneCollection created");
	}

	void SceneCollection::clear() {
		m_lights.clear();
		for (auto& collection : m_materialShaderCollections) {
			collection.clear();
		}
	}

	void SceneCollection::collect(Scene* pScene) {
		pScene->forEach<comp::Transform, comp::Model>([&](const comp::Transform& transform, const comp::Model& model) {
			sa::ModelAsset* pModel = model.model.getAsset();
			if(pModel)
				addObject(transform.getMatrix(), pModel);
		});
		pScene->forEach<comp::Light>([&](const comp::Light& light) {
			addLight(light.values);
		});
	}

	void SceneCollection::addObject(glm::mat4 transformation, ModelAsset* pModelAsset) {
		SA_PROFILE_FUNCTION();

		if (!pModelAsset || !pModelAsset->isLoaded())
			return;
		ModelData* pModel = &pModelAsset->data;

		ObjectData objectBuffer = {};
		objectBuffer.worldMat = transformation;

		// make sure all collection exists
		uint32_t i = 0;
		for (const auto& mesh : pModel->meshes) {
			Material* pMaterial = mesh.material.getAsset();
			MaterialShaderCollection& collection = getMaterialShaderCollection(pMaterial ? pMaterial->getMaterialShader().getAsset() : nullptr);
			collection.addMesh(pModelAsset, i, objectBuffer);
			i++;
		}
	}

	void SceneCollection::addLight(const LightData& light) {
		m_lights.push_back(light);
	}

	void SceneCollection::makeRenderReady() {
		SA_PROFILE_FUNCTION();
		

		//Ligths
		m_lightBuffer.clear();
		m_lightBuffer.write(static_cast<uint32_t>(m_lights.size()));
		m_lightBuffer.append(m_lights, 16);


		// Clear Dynamic buffers
		for (auto& collection : m_materialShaderCollections) {

			collection.m_objectBuffer.clear();
			collection.m_indirectIndexedBuffer.clear();
			collection.m_vertexBuffer.clear();
			collection.m_indexBuffer.clear();
			collection.m_materialBuffer.clear();
			collection.m_materialIndicesBuffer.clear();

			// reserve dynamic buffers
			{
				SA_PROFILE_SCOPE("Reserve rendering buffers");
				collection.m_objectBuffer.reserve(collection.m_objectCount * sizeof(ObjectData), IGNORE_CONTENT);
				collection.m_indirectIndexedBuffer.reserve(collection.m_uniqueMeshCount * sizeof(DrawIndexedIndirectCommand), IGNORE_CONTENT);
				collection.m_vertexBuffer.reserve(collection.m_vertexCount * sizeof(VertexNormalUV), IGNORE_CONTENT);
				collection.m_indexBuffer.reserve(collection.m_indexCount * sizeof(uint32_t), IGNORE_CONTENT);
				collection.m_materialBuffer.reserve(collection.m_uniqueMeshCount * sizeof(Material::Values), IGNORE_CONTENT);
				collection.m_materialIndicesBuffer.reserve(collection.m_uniqueMeshCount * sizeof(int32_t), IGNORE_CONTENT);
			}

			uint32_t firstInstance = 0;

			int32_t materialCount = 0;
			uint32_t meshCount = 0;

			for (size_t i = 0; i < collection.m_models.size(); i++) {
				ModelAsset* pModelAsset = collection.m_models.at(i);
				if(!pModelAsset->isLoaded())
					continue;
				for (const auto& objectBuffer : collection.m_objects[i]) {
					collection.m_objectBuffer << objectBuffer;
				}
				ModelData* pModel = &collection.m_models[i]->data;
				for (const auto& meshIndex : collection.m_meshes[i]) {
					const Mesh& mesh = pModel->meshes[meshIndex];
					uint32_t vertexOffset = collection.m_vertexBuffer.getElementCount<VertexNormalUV>();
					{
						SA_PROFILE_SCOPE("Append Vertex buffer");
						// Push mesh into buffers
						collection.m_vertexBuffer << mesh.vertices;
					}
					uint32_t firstIndex = collection.m_indexBuffer.getElementCount<uint32_t>();
					{
						SA_PROFILE_SCOPE("Append Index buffer");
						collection.m_indexBuffer << mesh.indices;
						
					}

					// Create a draw command for this mesh
					DrawIndexedIndirectCommand cmd = {};
					cmd.firstIndex = firstIndex;
					cmd.indexCount = mesh.indices.size();
					cmd.firstInstance = firstInstance;
					cmd.instanceCount = collection.m_objects[i].size();
					cmd.vertexOffset = vertexOffset;
					{
						SA_PROFILE_SCOPE("Append Draw buffer");
						collection.m_indirectIndexedBuffer << cmd;
					}
						
					//Material
					sa::Material* pMaterial = mesh.material.getAsset();
					if (pMaterial) {
						auto it = std::find(collection.m_materials.begin(), collection.m_materials.end(), pMaterial);
						if (it == collection.m_materials.end()) {
							uint32_t textureOffset = collection.m_textures.size();
							const std::vector<Texture>& matTextures = pMaterial->fetchTextures();
							collection.m_textures.insert(collection.m_textures.end(), matTextures.begin(), matTextures.end());

							Material::Values values = pMaterial->values;
							values.albedoMapFirst += textureOffset;
							values.normalMapFirst += textureOffset;
							values.metalnessMapFirst += textureOffset;
							values.roughnessMapFirst += textureOffset;
							values.emissiveMapFirst += textureOffset;
							values.occlusionMapFirst += textureOffset;

							collection.m_materials.push_back(pMaterial);
							collection.m_materialData.push_back(values);
							collection.m_materialIndices.push_back(materialCount);
							materialCount++;
						}
						else {
							collection.m_materialIndices.push_back(std::distance(collection.m_materials.begin(), it));
						}
					}
					else {
						collection.m_materialIndices.push_back(-1); // Default Material in shader
					}
					meshCount++;
				}
				firstInstance += collection.m_objects[i].size();
			}
			{
				SA_PROFILE_SCOPE("Write Materials");
				collection.m_materialBuffer.write(collection.m_materialData);
				collection.m_materialIndicesBuffer.write(collection.m_materialIndices);
			}
		}

	}

	void SceneCollection::swap() {
		m_lightBuffer.swap();

		for (auto& collection : m_materialShaderCollections) {
			collection.swap();
		}
	}
	
	const Buffer& SceneCollection::getLightBuffer() const {
		return m_lightBuffer.getBuffer();
	}

	std::vector<MaterialShaderCollection>::iterator SceneCollection::begin() {
		return m_materialShaderCollections.begin();
	}
	
	std::vector<MaterialShaderCollection>::iterator SceneCollection::end() {
		return m_materialShaderCollections.end();
	}
}