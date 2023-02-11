#include "pch.h"
#include "ForwardPlus.h"

#include "Engine.h"

namespace sa {
	void ForwardPlus::createPreDepthPass() {
		
		if (m_depthPreRenderProgram != NULL_RESOURCE) {
			m_renderer.destroyRenderProgram(m_depthPreRenderProgram);
			m_depthPreRenderProgram = NULL_RESOURCE;
		}
	
		m_depthPreRenderProgram = m_renderer.createRenderProgram()
			.addDepthAttachment(AttachmentFlagBits::eClear | AttachmentFlagBits::eSampled | AttachmentFlagBits::eStore)
			.beginSubpass()
			.addAttachmentReference(0, SubpassAttachmentUsage::DepthTarget)
			.endSubpass()
			.end();
	}
	
	void ForwardPlus::createLightCullingShader() {
		
		if (m_lightCullingPipeline != NULL_RESOURCE) {
			m_renderer.destroyPipeline(m_lightCullingPipeline);
			m_lightCullingPipeline = NULL_RESOURCE;
		}

		m_lightCullingPipeline = m_renderer.createComputePipeline((Engine::getShaderDirectory() / "LightCulling.comp.spv").generic_string());
		
	}

	void ForwardPlus::createColorPass() {
		
		if (m_colorRenderProgram != NULL_RESOURCE) {
			m_renderer.destroyRenderProgram(m_colorRenderProgram);
			m_colorRenderProgram = NULL_RESOURCE;
		}

		Format colorFormat = m_renderer.selectFormat({ Format::R32G32B32A32_SFLOAT }, TextureTypeFlagBits::COLOR_ATTACHMENT | TextureTypeFlagBits::SAMPLED);

		m_colorRenderProgram = m_renderer.createRenderProgram()
			.addColorAttachment(AttachmentFlagBits::eClear | AttachmentFlagBits::eStore | AttachmentFlagBits::eSampled, colorFormat)
			.addDepthAttachment(AttachmentFlagBits::eClear)
			.beginSubpass()
			.addAttachmentReference(0, sa::SubpassAttachmentUsage::ColorTarget)
			.addAttachmentReference(1, sa::SubpassAttachmentUsage::DepthTarget)
			.endSubpass()
			.end();
		m_renderer.setClearColor(m_colorRenderProgram, Color{ 0.0f, 0.0f, 0.1f, 1.0f });	
	}

	void ForwardPlus::initializeMainRenderData(RenderTarget::MainRenderData& data, Extent extent) {

		Format colorFormat = m_renderer.getAttachmentFormat(m_colorRenderProgram, 0);
		Format depthFormat = m_renderer.getAttachmentFormat(m_colorRenderProgram, 1);


		data.colorTexture = DynamicTexture2D(TextureTypeFlagBits::COLOR_ATTACHMENT | TextureTypeFlagBits::SAMPLED, extent, colorFormat);
		data.depthTexture = DynamicTexture2D(TextureTypeFlagBits::DEPTH_ATTACHMENT | TextureTypeFlagBits::SAMPLED, extent, depthFormat);


		//Depth pre pass
		data.depthFramebuffer = m_renderer.createFramebuffer(m_depthPreRenderProgram, { data.depthTexture });

		PipelineSettings settings = {};
		settings.dynamicStates.push_back(DynamicState::VIEWPORT);
		data.depthPipeline = m_renderer.createGraphicsPipeline(m_depthPreRenderProgram, 0, extent,
			(Engine::getShaderDirectory()  / "ForwardPlusColorPass.vert.spv").generic_string(), settings);
		
		data.sceneDepthDescriptorSet = m_renderer.allocateDescriptorSet(data.depthPipeline, 0);

		// Light culling pass
		data.tileCount = { extent.width, extent.height };
		data.tileCount += (TILE_SIZE - data.tileCount % TILE_SIZE);
		data.tileCount /= TILE_SIZE;

		size_t totalTileCount = data.tileCount.x * data.tileCount.y;
		if (data.lightCullingDescriptorSet == NULL_RESOURCE) 
			data.lightCullingDescriptorSet = m_renderer.allocateDescriptorSet(m_lightCullingPipeline, 0);

		data.lightIndexBuffer = m_renderer.createDynamicBuffer(BufferType::STORAGE, sizeof(uint32_t) * MAX_LIGHTS_PER_TILE * totalTileCount);


		// Color pass
		data.colorFramebuffer = m_renderer.createFramebuffer(m_colorRenderProgram, { data.colorTexture, data.depthTexture });
		data.colorPipeline = m_renderer.createGraphicsPipeline(m_colorRenderProgram, 0, extent,
			(Engine::getShaderDirectory()  / "ForwardPlusColorPass.vert.spv").generic_string(), (Engine::getShaderDirectory() / "ForwardPlusColorPass.frag.spv").generic_string(), settings);

		data.sceneDescriptorSet = m_renderer.allocateDescriptorSet(data.colorPipeline, SET_PER_FRAME);


		m_renderer.updateDescriptorSet(data.sceneDepthDescriptorSet, 0, m_objectBuffer);

		m_renderer.updateDescriptorSet(data.lightCullingDescriptorSet, 0, data.depthTexture, m_linearSampler);	// read depth texture
		m_renderer.updateDescriptorSet(data.lightCullingDescriptorSet, 1, data.lightIndexBuffer);				// write what lights are in what tiles
		m_renderer.updateDescriptorSet(data.lightCullingDescriptorSet, 2, m_lightBuffer);						// light data

		m_renderer.updateDescriptorSet(data.sceneDescriptorSet, 0, m_objectBuffer);
		m_renderer.updateDescriptorSet(data.sceneDescriptorSet, 1, m_lightBuffer);
		m_renderer.updateDescriptorSet(data.sceneDescriptorSet, 2, m_materialBuffer);
		m_renderer.updateDescriptorSet(data.sceneDescriptorSet, 3, m_materialIndicesBuffer);
		m_renderer.updateDescriptorSet(data.sceneDescriptorSet, 4, data.lightIndexBuffer);
		m_renderer.updateDescriptorSet(data.sceneDescriptorSet, 5, m_linearSampler);

	}

	
	void ForwardPlus::collectMeshes(Scene* pScene) {
		SA_PROFILE_FUNCTION();
		
		m_objectBuffer.clear();
		m_indirectIndexedBuffer.clear();
		m_vertexBuffer.clear();
		m_indexBuffer.clear();


		m_materialData.clear();
		m_materialIndices.clear();

		m_textures.clear();

		uint32_t objectCount = 0;
		uint32_t vertexCount = 0;
		uint32_t indexCount = 0;
		uint32_t uniqueMeshCount = 0;

		m_models.clear();
		pScene->forEach<comp::Transform, comp::Model>([&](const comp::Transform& transform, const comp::Model& model) {
			ModelAsset* modelAsset = AssetManager::get().getAsset<ModelAsset>(model.modelID);
			if (!modelAsset || !modelAsset->isLoaded())
				return;
			ModelData* pModel = &modelAsset->data;

			ObjectData objectBuffer = {};
			objectBuffer.worldMat = transform.getMatrix();
			
			auto it = std::find(m_models.begin(), m_models.end(), pModel);
			if (it == m_models.end()) {
				m_models.push_back(pModel);
				if (m_objects.size() >= m_models.size()) {
					m_objects[m_models.size() - 1].clear();
					m_objects[m_models.size() - 1].push_back(objectBuffer);
				}
				else {
					m_objects.push_back({ objectBuffer });
				}
				for (const auto& mesh : pModel->meshes) {
					vertexCount += mesh.vertices.size();
					indexCount += mesh.indices.size();
					uniqueMeshCount++;
				}
			}
			else {
				m_objects[std::distance(m_models.begin(), it)].push_back(objectBuffer);
			}
			objectCount++;
		});

		m_objectBuffer.reserve(				objectCount * sizeof(ObjectData),						IGNORE_CONTENT);
		m_vertexBuffer.reserve(				vertexCount * sizeof(VertexNormalUV),					IGNORE_CONTENT);
		m_indexBuffer.reserve(				indexCount * sizeof(uint32_t),							IGNORE_CONTENT);
		m_indirectIndexedBuffer.reserve(	uniqueMeshCount * sizeof(DrawIndexedIndirectCommand),	IGNORE_CONTENT);

		uint32_t firstInstance = 0;

		uint32_t materialCount = 0;
		uint32_t meshCount = 0;

		m_materials.clear();

		for (size_t i = 0; i < m_models.size(); i++) {
			for (const auto& objectBuffer : m_objects[i]) {
				m_objectBuffer << objectBuffer;
			}
			ModelData* pModel = m_models[i];

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
				MaterialAsset* materialAsset = AssetManager::get().getAsset<MaterialAsset>(mesh.materialID);
				Material* pMaterial = nullptr;
				if(materialAsset)
					pMaterial = &materialAsset->data;
				if (pMaterial && materialAsset->isLoaded()) {
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
				meshCount++;
				

			}
			firstInstance += m_objects[i].size();
		}
		
		m_materialBuffer.write(m_materialData);
		m_materialIndicesBuffer.write(m_materialIndices);

		
	}

	void ForwardPlus::onWindowResize(Extent extent) {
		
	}

	void ForwardPlus::init() {
		
		createPreDepthPass();
		createLightCullingShader();
		createColorPass();

		// Samplers
		m_linearSampler = m_renderer.createSampler(FilterMode::LINEAR);
		m_nearestSampler = m_renderer.createSampler(FilterMode::NEAREST);

		// Create buffers
		uint32_t lightCount = 0U;
		m_lightBuffer = m_renderer.createDynamicBuffer(BufferType::STORAGE, sizeof(uint32_t), &lightCount);

		m_indirectIndexedBuffer = m_renderer.createDynamicBuffer(BufferType::INDIRECT);
		m_vertexBuffer = m_renderer.createDynamicBuffer(BufferType::VERTEX);
		m_indexBuffer = m_renderer.createDynamicBuffer(BufferType::INDEX);
		m_objectBuffer = m_renderer.createDynamicBuffer(BufferType::STORAGE);

		m_materialBuffer = m_renderer.createDynamicBuffer(BufferType::STORAGE);
		m_materialIndicesBuffer = m_renderer.createDynamicBuffer(BufferType::STORAGE);

		
		//DEBUG
		/*
		m_debugLightHeatmap = DynamicTexture2D(TextureTypeFlagBits::COLOR_ATTACHMENT | TextureTypeFlagBits::SAMPLED, { m_tileCount.x, m_tileCount.y });
		m_debugLightHeatmapRenderProgram = m_renderer.createRenderProgram()
			.addColorAttachment(true, m_debugLightHeatmap)
			.beginSubpass()
			.addAttachmentReference(0, SubpassAttachmentUsage::ColorTarget)
			.endSubpass()
			.end();

		m_debugLightHeatmapFramebuffer = m_renderer.createFramebuffer(m_debugLightHeatmapRenderProgram, { m_debugLightHeatmap });
		m_debugLightHeatmapPipeline = m_renderer.createGraphicsPipeline(m_debugLightHeatmapRenderProgram, 0, { m_tileCount.x, m_tileCount.y },
			(Engine::getShaderDirectory()  / "DebugHeatmap.vert.spv",
			"../Engine/shaders/DebugHeatmap.frag.spv"
			);
		m_debugLightHeatmapDescriptorSet = m_renderer.allocateDescriptorSet(m_debugLightHeatmapPipeline, 0);
		m_renderer.updateDescriptorSet(m_debugLightHeatmapDescriptorSet, 0, m_lightIndexBuffer);

		setShowHeatmap(false);
		*/



	}

	void ForwardPlus::cleanup() {
	
	}

	void ForwardPlus::updateData(RenderContext& context) {
		
	}

	bool ForwardPlus::prepareRender(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget) {
		if (!pCamera)
			return false;

		RenderTarget::MainRenderData& data = pRenderTarget->mainRenderData;
		
		if (!data.isInitialized) {
			pRenderTarget->cleanupMainRenderData();
			initializeMainRenderData(data, pRenderTarget->extent);
			data.isInitialized = true;
		}


		context.updateDescriptorSet(data.sceneDepthDescriptorSet, 0, m_objectBuffer);
		context.updateDescriptorSet(data.sceneDescriptorSet, 0, m_objectBuffer);
		context.updateDescriptorSet(data.sceneDescriptorSet, 1, m_lightBuffer);
		context.updateDescriptorSet(data.sceneDescriptorSet, 2, m_materialBuffer);
		context.updateDescriptorSet(data.sceneDescriptorSet, 3, m_materialIndicesBuffer);
		context.updateDescriptorSet(data.sceneDescriptorSet, 4, data.lightIndexBuffer);
		context.updateDescriptorSet(data.sceneDescriptorSet, 6, m_textures);

		context.updateDescriptorSet(data.lightCullingDescriptorSet, 1, data.lightIndexBuffer);
		context.updateDescriptorSet(data.lightCullingDescriptorSet, 2, m_lightBuffer);



		context.bindVertexBuffers(0, { m_vertexBuffer });
		context.bindIndexBuffer(m_indexBuffer);

		Matrix4x4 projViewMat = pCamera->getProjectionMatrix() * pCamera->getViewMatrix();

		PerFrameBuffer perFrame;
		perFrame.projViewMatrix = projViewMat;
		perFrame.viewPos = pCamera->getPosition();

		// Depth prepass
		context.beginRenderProgram(m_depthPreRenderProgram, data.depthFramebuffer, SubpassContents::DIRECT);
		context.bindPipeline(data.depthPipeline);
		
		context.setViewport(pCamera->getViewport());

		context.bindDescriptorSet(data.sceneDepthDescriptorSet, data.depthPipeline);


		if (m_indirectIndexedBuffer.getElementCount<DrawIndexedIndirectCommand>() > 0) {
			context.pushConstant(data.depthPipeline, ShaderStageFlagBits::VERTEX, perFrame);
			context.drawIndexedIndirect(m_indirectIndexedBuffer, 0, m_indirectIndexedBuffer.getElementCount<DrawIndexedIndirectCommand>(), sizeof(DrawIndexedIndirectCommand));
		}

		context.endRenderProgram(m_depthPreRenderProgram);


		// Light culling
		context.bindPipeline(m_lightCullingPipeline);
		context.bindDescriptorSet(data.lightCullingDescriptorSet, m_lightCullingPipeline);

		context.pushConstant(m_lightCullingPipeline, ShaderStageFlagBits::COMPUTE, pCamera->getProjectionMatrix());
		context.pushConstant(m_lightCullingPipeline, ShaderStageFlagBits::COMPUTE, pCamera->getViewMatrix(), sizeof(Matrix4x4));

		context.dispatch(data.tileCount.x, data.tileCount.y, 1);

		//context.barrierColorCompute(m_lightBuffer);
		// TODO generate shadowMaps


		return true;
	}

	void ForwardPlus::render(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget) {
		if (!pCamera)
			return;
		RenderTarget::MainRenderData& data = pRenderTarget->mainRenderData;

		Matrix4x4 projViewMat = pCamera->getProjectionMatrix() * pCamera->getViewMatrix();

		PerFrameBuffer perFrame;
		perFrame.projViewMatrix = projViewMat;
		perFrame.viewPos = pCamera->getPosition();

		// Main color pass
		context.beginRenderProgram(m_colorRenderProgram, data.colorFramebuffer, SubpassContents::DIRECT);
		context.bindPipeline(data.colorPipeline);
		context.bindDescriptorSet(data.sceneDescriptorSet, data.colorPipeline);

		context.setViewport(pCamera->getViewport());

		if (m_indirectIndexedBuffer.getElementCount<DrawIndexedIndirectCommand>() > 0) {
			context.pushConstant(data.colorPipeline, ShaderStageFlagBits::VERTEX | ShaderStageFlagBits::FRAGMENT, perFrame);
			context.pushConstant(data.colorPipeline, ShaderStageFlagBits::FRAGMENT, data.tileCount.x, sizeof(perFrame));

			context.drawIndexedIndirect(m_indirectIndexedBuffer, 0, m_indirectIndexedBuffer.getElementCount<DrawIndexedIndirectCommand>(), sizeof(DrawIndexedIndirectCommand));
		}

		context.endRenderProgram(m_colorRenderProgram);

		
		/*
		context.beginRenderProgram(m_debugLightHeatmapRenderProgram, m_debugLightHeatmapFramebuffer, SubpassContents::DIRECT);
		context.bindPipeline(m_debugLightHeatmapPipeline);
		context.bindDescriptorSet(m_debugLightHeatmapDescriptorSet, m_debugLightHeatmapPipeline);
		context.pushConstant(m_debugLightHeatmapPipeline, ShaderStageFlagBits::FRAGMENT, m_tileCount.x);
		context.draw(6, 1);
		context.endRenderProgram(m_debugLightHeatmapRenderProgram);
		*/

	}

	void ForwardPlus::endRender(RenderContext& context) {
		
		m_lightBuffer.swap();
		m_vertexBuffer.swap();
		m_indexBuffer.swap();
		m_indirectIndexedBuffer.swap();
		m_objectBuffer.swap();
		m_materialBuffer.swap();
		m_materialIndicesBuffer.swap();
	
	}

	void ForwardPlus::updateLights(Scene* pScene) {
		SA_PROFILE_FUNCTION();
		
		m_lights.clear();

		pScene->forEach<comp::Transform, comp::Light>([](const comp::Transform& transform, comp::Light& light) {
			light.values.position = glm::vec4(transform.position, light.values.position.w);
			light.values.direction = glm::vec4(transform.rotation * glm::vec3(0, 0, 1), light.values.direction.w);
		});

		pScene->forEach<comp::Light>([&](const comp::Light& light) {
			m_lights.push_back(light.values);
		});
		
		m_lightBuffer.write(static_cast<uint32_t>(m_lights.size()));
		m_lightBuffer.append(m_lights, 16);
	}

	const Texture2D& ForwardPlus::getLightHeatmap() const {
		return m_debugLightHeatmap;
	}

	void ForwardPlus::setShowHeatmap(bool value) {
		
	}


}