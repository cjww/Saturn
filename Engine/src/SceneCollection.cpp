#include "pch.h"
#include "Graphics/SceneCollection.h"

#include "Scene.h"

namespace sa {
	MaterialShaderCollection::MaterialShaderCollection(MaterialShader* pMaterialShader) {
		m_materialShaderID = pMaterialShader->getID();
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

	void MaterialShaderCollection::addMesh(ModelAsset* pModelAsset, uint32_t meshIndex, const Entity& entity) {

		auto it = std::find(m_models.begin(), m_models.end(), pModelAsset);
		size_t modelIndex = std::distance(m_models.begin(), it);
		bool uniqueModel = it == m_models.end();
		if (uniqueModel) {
			modelIndex = m_models.size();
			m_models.emplace_back(pModelAsset);
			m_meshes.push_back({});
			if (m_objects.size() >= m_models.size()) {
				m_objects[modelIndex].clear();
				m_objects[modelIndex].push_back(entity);
			}
			else {
				m_objects.push_back({ entity });
			}
		}
		else {
			m_objects[modelIndex].push_back(entity);
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

	void MaterialShaderCollection::removeMesh(const ModelAsset* pModelAsset, uint32_t meshIndex, const Entity& entity) {
		const auto modelIt = std::find(m_models.begin(), m_models.end(), pModelAsset);
		if (modelIt == m_models.end())
			throw std::runtime_error("Model not found in collection");
		const size_t modelIndex = std::distance(m_models.begin(), modelIt);

		std::erase(m_objects[modelIndex], entity);
		m_objectCount--;
		if(m_objects[modelIndex].empty()) { // if erased every object using this model
			m_models.erase(modelIt); // remove model
			for(const auto& index : m_meshes[modelIndex]) {
				const Mesh& mesh = pModelAsset->data.meshes[index];
				m_vertexCount -= mesh.vertices.size();
				m_indexCount -= mesh.indices.size();
				m_uniqueMeshCount--;
			}
			m_meshes.erase(m_meshes.begin() + modelIndex); // erase all meshes connected to model
			m_objects.erase(m_objects.begin() + modelIndex); // erase vector that was empty
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

	bool MaterialShaderCollection::readyDescriptorSets() {
		const auto pMaterialShader = getMaterialShader();
		if (!pMaterialShader || !pMaterialShader->isLoaded())
			return false;

		if (m_sceneDescriptorSetColorPass == NULL_RESOURCE || !pMaterialShader->m_colorShaderSet.hasAllocatedDescriptorSet(m_sceneDescriptorSetColorPass)) {
			m_sceneDescriptorSetColorPass = pMaterialShader->m_colorShaderSet.allocateDescriptorSet(SET_PER_FRAME);
		}

		if (m_sceneDescriptorSetDepthPass == NULL_RESOURCE || !pMaterialShader->m_depthShaderSet.hasAllocatedDescriptorSet(m_sceneDescriptorSetDepthPass)) {
			m_sceneDescriptorSetDepthPass = pMaterialShader->m_depthShaderSet.allocateDescriptorSet(SET_PER_FRAME);
		}

		return true;
	}


	void MaterialShaderCollection::recreatePipelines(ResourceID colorRenderProgram, ResourceID depthRenderProgram, Extent extent) {
		const auto pMaterialShader = getMaterialShader();
		if (!pMaterialShader)
			return;
		if (extent == m_currentExtent && !pMaterialShader->m_recompiled)
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

		m_colorPipeline = renderer.createGraphicsPipeline(colorRenderProgram, 0, extent, pMaterialShader->m_colorShaderSet, settings);
		m_depthPipeline = renderer.createGraphicsPipeline(depthRenderProgram, 0, extent, pMaterialShader->m_depthShaderSet, settings);
		m_currentExtent = extent;

		pMaterialShader->m_recompiled = false;
	}

	void MaterialShaderCollection::bindColorPipeline(RenderContext& context) {
		context.bindPipeline(m_colorPipeline);
	}

	void MaterialShaderCollection::bindDepthPipeline(RenderContext& context) {
		context.bindPipeline(m_depthPipeline);
	}

	
	MaterialShaderCollection& SceneCollection::getMaterialShaderCollection(MaterialShader* pMaterialShader) {
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
		return AssetManager::get().getAsset<MaterialShader>(m_materialShaderID);
	}

	void SceneCollection::addQueuedEntities() {
		for (auto it = m_entitiesToAdd.begin(); it != m_entitiesToAdd.end();) {
			const Entity& entity = *it;

			if (m_entityModels.count(entity)) { // if already added, remove old one
				ModelAsset* pModelAsset = AssetManager::get().getAsset<ModelAsset>(m_entityModels[entity]);
				removeObject(entity, pModelAsset);
			}

			ModelAsset* pModelAsset = entity.getComponent<comp::Model>()->model.getAsset();
			if (pModelAsset) { // if has asset and is done loading
				addObject(entity, pModelAsset);
				m_entityModels[entity] = pModelAsset->getID();
				it = m_entitiesToAdd.erase(it);
				continue;
			}

			it++;
		}
	}

	void SceneCollection::onModelConstruct(const scene_event::ComponentCreated<comp::Model>& e) {
		m_entitiesToAdd.insert(e.entity);
	}

	void SceneCollection::onModelUpdate(const scene_event::ComponentUpdated<comp::Model>& e) {
		m_entitiesToAdd.insert(e.entity);
	}

	void SceneCollection::onModelDestroy(const scene_event::ComponentDestroyed<comp::Model>& e) {
		m_entitiesToAdd.erase(e.entity);

		if (m_entityModels.count(e.entity)) {
			sa::ModelAsset* pModelAsset = AssetManager::get().getAsset<sa::ModelAsset>(m_entityModels[e.entity]);
			removeObject(e.entity, pModelAsset);
			m_entityModels.erase(e.entity);
		}
	}

	void SceneCollection::onLightConstruct(const scene_event::ComponentCreated<comp::Light>& e) {
		if (m_entityLights.count(e.entity)) {
			const LightData& data = m_entityLights[e.entity];
			removeLight(data);
			m_entityLights.erase(e.entity);
		}
		const auto& data = e.entity.getComponent<comp::Light>()->values;
		addLight(data);
		m_entityLights[e.entity] = data;
	}

	void SceneCollection::onLightUpdate(const scene_event::ComponentUpdated<comp::Light>& e) {
		if (m_entityLights.count(e.entity)) {
			const LightData& data = m_entityLights[e.entity];
			removeLight(data);
			m_entityLights.erase(e.entity);
		}
		const auto& data = e.entity.getComponent<comp::Light>()->values;
		addLight(data);
		m_entityLights[e.entity] = data;
	}

	void SceneCollection::onLightDestroy(const scene_event::ComponentDestroyed<comp::Light>& e) {
		if (m_entityLights.count(e.entity)) {
			const LightData& data = m_entityLights[e.entity];
			removeLight(data);
			m_entityLights.erase(e.entity);
		}
	}

	SceneCollection::SceneCollection(CollectionMode mode)
		: m_mode(mode)
	{
		sa::Renderer& renderer = sa::Renderer::get();

		uint32_t lightCount = 0U;
		m_lightBuffer = renderer.createDynamicBuffer(BufferType::STORAGE, sizeof(uint32_t), &lightCount);

		SA_DEBUG_LOG_INFO("SceneCollection created");
	}

	SceneCollection::~SceneCollection() {
		stopListen();
	}

	void SceneCollection::clear() {
		m_lights.clear();
		for (auto& collection : m_materialShaderCollections) {
			collection.clear();
		}
	}

	void SceneCollection::collect(Scene* pScene) {
		if (m_mode != CollectionMode::CONTINUOUS)
			throw std::runtime_error("Mode has to be CONTINUOUS to call collect");
		
		pScene->forEach<comp::Transform, comp::Model>([&](const Entity& entity, const comp::Transform& transform, const comp::Model& model) {
			sa::ModelAsset* pModel = model.model.getAsset();
			if(pModel)
				addObject(entity, pModel);
		});
		pScene->forEach<comp::Light>([&](const comp::Light& light) {
			addLight(light.values);
		});
	}

	void SceneCollection::listen(Scene* pScene) {
		if (m_mode != CollectionMode::REACTIVE)
			throw std::runtime_error("Mode has to be REACTIVE to call listen");
		stopListen();
		m_connections.emplace_back(pScene->sink<scene_event::ComponentCreated<comp::Model>>().connect<&SceneCollection::onModelConstruct>(this));
		m_connections.emplace_back(pScene->sink<scene_event::ComponentDestroyed<comp::Model>>().connect<&SceneCollection::onModelDestroy>(this));
		m_connections.emplace_back(pScene->sink<scene_event::ComponentUpdated<comp::Model>>().connect<&SceneCollection::onModelUpdate>(this));

		m_connections.emplace_back(pScene->sink<scene_event::ComponentCreated<comp::Light>>().connect<&SceneCollection::onLightConstruct>(this));
		m_connections.emplace_back(pScene->sink<scene_event::ComponentDestroyed<comp::Light>>().connect<&SceneCollection::onLightDestroy>(this));
		m_connections.emplace_back(pScene->sink<scene_event::ComponentUpdated<comp::Light>>().connect<&SceneCollection::onLightUpdate>(this));

	}

	void SceneCollection::stopListen() {
		for (auto conn : m_connections) {
			conn.release();
		}
		m_connections.clear();
	}

	void SceneCollection::setMode(CollectionMode mode) {
		m_mode = mode;
		switch (m_mode) {
		case CollectionMode::CONTINUOUS:
			stopListen();
			break;
		default:
			break;
		}
	}

	SceneCollection::CollectionMode SceneCollection::getMode() const {
		return m_mode;
	}

	void SceneCollection::addObject(const Entity& entity, ModelAsset* pModelAsset) {
		SA_PROFILE_FUNCTION();

		if (!pModelAsset || !pModelAsset->isLoaded())
			return;
		ModelData* pModel = &pModelAsset->data;

		// make sure all collection exists
		uint32_t i = 0;
		for (const auto& mesh : pModel->meshes) {
			Material* pMaterial = mesh.material.getAsset();

			MaterialShader* pMaterialShader;
			if(pMaterial && pMaterial->getMaterialShader().getAsset()) {
				pMaterialShader = pMaterial->getMaterialShader().getAsset();
			}
			else {
				pMaterialShader = AssetManager::get().getDefaultMaterialShader();
			}
			MaterialShaderCollection& collection = getMaterialShaderCollection(pMaterialShader);
			collection.addMesh(pModelAsset, i, entity);
			i++;
		}
	}

	void SceneCollection::addLight(const LightData& light) {
		m_lights.push_back(light);
	}

	void SceneCollection::removeObject(const Entity& entity, ModelAsset* pModelAsset) {
		SA_PROFILE_FUNCTION();
		if (!pModelAsset || !pModelAsset->isLoaded())
			return;
		ModelData* pModel = &pModelAsset->data;

		
		uint32_t i = 0;
		for (const auto& mesh : pModel->meshes) {
			Material* pMaterial = mesh.material.getAsset();

			MaterialShader* pMaterialShader;
			if (pMaterial && pMaterial->getMaterialShader().getAsset()) {
				pMaterialShader = pMaterial->getMaterialShader().getAsset();
			}
			else {
				pMaterialShader = AssetManager::get().getDefaultMaterialShader();
			}
			MaterialShaderCollection& collection = getMaterialShaderCollection(pMaterialShader);
			collection.removeMesh(pModelAsset, i, entity);
			i++;
		}
	}

	void SceneCollection::removeLight(const LightData& light) {
		std::erase(m_lights, light);
	}

	void SceneCollection::makeRenderReady() {
		SA_PROFILE_FUNCTION();
		if(m_mode == CollectionMode::REACTIVE) {
			addQueuedEntities();
		}


		//Ligths
		m_lightBuffer.clear();
		m_lightBuffer.write(static_cast<uint32_t>(m_lights.size()));
		m_lightBuffer.append(m_lights, 16);


		for (auto& collection : m_materialShaderCollections) {

			// Clear Dynamic buffers
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
				for (const auto& entity : collection.m_objects[i]) {
					auto pTransform = entity.getComponent<comp::Transform>();
					if(!pTransform) {
						collection.m_objectBuffer << glm::mat4(1);
						continue;
					}
					collection.m_objectBuffer << pTransform->getMatrix();
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