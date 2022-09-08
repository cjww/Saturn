#include "pch.h"
#include "ForwardPlus.h"

namespace sa {


	void ForwardPlus::createTextures(sa::Extent extent) {
		if (m_colorTexture.isValid()) m_colorTexture.destroy();
		if (m_depthTexture.isValid()) m_depthTexture.destroy();
		if (m_outputTexture.isValid()) m_outputTexture.destroy();


		m_colorTexture = m_renderer.createTexture2D(sa::TextureTypeFlagBits::COLOR_ATTACHMENT | sa::TextureTypeFlagBits::SAMPLED, extent);
		m_depthTexture = m_renderer.createTexture2D(sa::TextureTypeFlagBits::DEPTH_ATTACHMENT | sa::TextureTypeFlagBits::SAMPLED, extent);
		
		m_outputTexture = m_renderer.createTexture2D(sa::TextureTypeFlagBits::COLOR_ATTACHMENT | sa::TextureTypeFlagBits::SAMPLED, extent);
	}

	void ForwardPlus::createRenderPasses() {
		m_colorRenderProgram = m_renderer.createRenderProgram()
			.addColorAttachment(true, m_colorTexture)
			.addDepthAttachment(m_depthTexture)
			.beginSubpass()
			.addAttachmentReference(0, sa::SubpassAttachmentUsage::ColorTarget)
			.addAttachmentReference(1, sa::SubpassAttachmentUsage::DepthTarget)
			.endSubpass()
			.end();


		auto composeProgramFactory = m_renderer.createRenderProgram();
		if (m_useImGui) {
			composeProgramFactory.addColorAttachment(true, m_outputTexture);
		}
		else {
			composeProgramFactory.addSwapchainAttachment(m_pWindow->getSwapchainID());
		}

		m_composeRenderProgram = composeProgramFactory.beginSubpass()
			.addAttachmentReference(0, SubpassAttachmentUsage::ColorTarget)
			.endSubpass()
			.addSubpassDependency()
			.end();


		m_depthPreRenderProgram = m_renderer.createRenderProgram()
			.addDepthAttachment(m_depthTexture, true)
			.beginSubpass()
			.addAttachmentReference(0, SubpassAttachmentUsage::DepthTarget)
			.endSubpass()
			.end();
	}

	void ForwardPlus::createFramebuffers(sa::Extent extent) {
		if (m_colorFramebuffer != NULL_RESOURCE)
			m_renderer.destroyFramebuffer(m_colorFramebuffer);
	
		m_colorFramebuffer = m_renderer.createFramebuffer(m_colorRenderProgram, { m_colorTexture, m_depthTexture });
	
		if (m_composeFramebuffer != NULL_RESOURCE)
			m_renderer.destroyFramebuffer(m_composeFramebuffer);

		if (m_useImGui) {
			m_composeFramebuffer = m_renderer.createFramebuffer(m_composeRenderProgram, { m_outputTexture });
		}
		else {
			m_composeFramebuffer = m_renderer.createSwapchainFramebuffer(m_composeRenderProgram, m_pWindow->getSwapchainID(), {});
		}

		m_depthPreFramebuffer = m_renderer.createFramebuffer(m_depthPreRenderProgram, { m_depthTexture });

	}

	void ForwardPlus::createPipelines(Extent extent) {
		if (m_colorPipeline != NULL_RESOURCE)
			m_renderer.destroyPipeline(m_colorPipeline);

		PipelineSettings settings = {};
		settings.dynamicStates.push_back(DynamicState::VIEWPORT);

		m_colorPipeline = m_renderer.createGraphicsPipeline(m_colorRenderProgram, 0, extent,
			"../Engine/shaders/ForwardPlusColorPass.vert.spv", "../Engine/shaders/ForwardPlusColorPass.frag.spv", settings);
	
		if (m_composePipeline != NULL_RESOURCE)
			m_renderer.destroyPipeline(m_composePipeline);

		m_composePipeline = m_renderer.createGraphicsPipeline(m_composeRenderProgram, 0, extent,
				"../Engine/shaders/Compose.vert.spv", "../Engine/shaders/Compose.frag.spv");


		m_depthPrePipeline = m_renderer.createGraphicsPipeline(m_depthPreRenderProgram, 0, extent,
			"../Engine/shaders/ForwardPlusColorPass.vert.spv", settings);
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

		pScene->view<comp::Transform, comp::Model>().each([&](const comp::Transform& transform, const comp::Model& model) {
			ModelData* pModel = AssetManager::get().getModel(model.modelID);
			if (!pModel)
				return;

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

		//MaterialBuffer materialBuffer = {};

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
				Material* pMaterial = AssetManager::get().getMaterial(mesh.materialID);
				if (pMaterial) {
					auto it = std::find(m_materials.begin(), m_materials.end(), pMaterial);
					if (it == m_materials.end()) {
						uint32_t textureOffset = m_textures.size();
						const std::vector<Texture>& matTextures = pMaterial->getTextures();
						m_textures.insert(m_textures.end(), matTextures.begin(), matTextures.end());
					
						Material::Values values = pMaterial->values;
						values.diffuseMapFirst = values.diffuseMapFirst + textureOffset;
						values.emissiveMapFirst = values.emissiveMapFirst + textureOffset;
						values.lightMapFirst = values.lightMapFirst + textureOffset;
						values.normalMapFirst = values.normalMapFirst + textureOffset;
						values.specularMapFirst = values.specularMapFirst + textureOffset;
						//m_materialBuffer << values;

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

	void ForwardPlus::init(sa::RenderWindow* pWindow, bool setupImGui) {
		
		m_useImGui = setupImGui;
		m_pWindow = pWindow;
		sa::Extent extent = m_pWindow->getCurrentExtent();

		createTextures(extent);
		createRenderPasses();
		createFramebuffers(extent);
		createPipelines(extent);

		if (setupImGui) {
			setupImGuiPass();
		}

		m_linearSampler = m_renderer.createSampler(FilterMode::LINEAR);
		m_nearestSampler = m_renderer.createSampler(FilterMode::NEAREST);

		uint32_t lightCount = 0U;
		m_lightBuffer = m_renderer.createDynamicBuffer(BufferType::STORAGE, sizeof(uint32_t), &lightCount);

		m_sceneDescriptorSet = m_renderer.allocateDescriptorSet(m_colorPipeline, SET_PER_FRAME);


		m_composeDescriptorSet = m_renderer.allocateDescriptorSet(m_composePipeline, 0);
		m_renderer.updateDescriptorSet(m_composeDescriptorSet, 0, m_colorTexture, m_linearSampler);
	
		m_indirectIndexedBuffer = m_renderer.createDynamicBuffer(BufferType::INDIRECT);
		m_vertexBuffer = m_renderer.createDynamicBuffer(BufferType::VERTEX);
		m_indexBuffer = m_renderer.createDynamicBuffer(BufferType::INDEX);
		m_objectBuffer = m_renderer.createDynamicBuffer(BufferType::STORAGE);

		m_materialBuffer = m_renderer.createDynamicBuffer(BufferType::STORAGE);
		m_materialIndicesBuffer = m_renderer.createDynamicBuffer(BufferType::STORAGE);

		m_renderer.updateDescriptorSet(m_sceneDescriptorSet, 0, m_objectBuffer);
		m_renderer.updateDescriptorSet(m_sceneDescriptorSet, 1, m_lightBuffer);
		m_renderer.updateDescriptorSet(m_sceneDescriptorSet, 2, m_materialBuffer);
		m_renderer.updateDescriptorSet(m_sceneDescriptorSet, 5, m_linearSampler);
		
		m_sceneDepthDescriptorSet = m_renderer.allocateDescriptorSet(m_depthPrePipeline, 0);
		m_renderer.updateDescriptorSet(m_sceneDepthDescriptorSet, 0, m_objectBuffer);

		m_lightCullingPipeline = m_renderer.createComputePipeline("../Engine/shaders/LightCulling2.comp.spv");
		m_lightCullingDescriptorSet = m_renderer.allocateDescriptorSet(m_lightCullingPipeline, 0);
		m_renderer.updateDescriptorSet(m_lightCullingDescriptorSet, 0, m_depthTexture, m_linearSampler);

		m_tileCount = m_pWindow->getCurrentExtent();
		m_tileCount += m_tileCount % TILE_SIZE;
		m_tileCount /= TILE_SIZE;
		
		size_t totalTileCount = m_tileCount.x * m_tileCount.y;

		m_lightIndexBuffer = m_renderer.createBuffer(BufferType::STORAGE, sizeof(uint32_t) * MAX_LIGHTS_PER_TILE * totalTileCount);
		m_renderer.updateDescriptorSet(m_lightCullingDescriptorSet, 1, m_lightIndexBuffer);
		m_renderer.updateDescriptorSet(m_lightCullingDescriptorSet, 2, m_lightBuffer);
		m_renderer.updateDescriptorSet(m_sceneDescriptorSet, 4, m_lightIndexBuffer);

		//DEBUG
		m_debugLightHeatmap = m_renderer.createTexture2D(TextureTypeFlagBits::COLOR_ATTACHMENT | TextureTypeFlagBits::SAMPLED, { m_tileCount.x, m_tileCount.y });
		m_debugLightHeatmapRenderProgram = m_renderer.createRenderProgram()
			.addColorAttachment(true, m_debugLightHeatmap)
			.beginSubpass()
			.addAttachmentReference(0, SubpassAttachmentUsage::ColorTarget)
			.endSubpass()
			.end();

		m_debugLightHeatmapFramebuffer = m_renderer.createFramebuffer(m_debugLightHeatmapRenderProgram, { m_debugLightHeatmap });
		m_debugLightHeatmapPipeline = m_renderer.createGraphicsPipeline(m_debugLightHeatmapRenderProgram, 0, { m_tileCount.x, m_tileCount.y },
			"../Engine/shaders/DebugHeatmap.vert.spv",
			"../Engine/shaders/DebugHeatmap.frag.spv"
			);
		m_debugLightHeatmapDescriptorSet = m_renderer.allocateDescriptorSet(m_debugLightHeatmapPipeline, 0);
		m_renderer.updateDescriptorSet(m_debugLightHeatmapDescriptorSet, 0, m_lightIndexBuffer);

		m_renderer.updateDescriptorSet(m_composeDescriptorSet, 1, *AssetManager::get().loadDefaultBlackTexture(), m_nearestSampler);
		
		m_renderer.setClearColor(m_colorRenderProgram, Color{ 0.3f, 0.3f, 0.3f });

	}

	void ForwardPlus::cleanup() {
		/*
		if (m_colorFramebuffer != NULL_RESOURCE)
			m_renderer.destroyFramebuffer(m_colorFramebuffer);

		if (m_composeFramebuffer != NULL_RESOURCE)
			m_renderer.destroyFramebuffer(m_composeFramebuffer);

		if (m_colorTexture.isValid()) m_colorTexture.destroy();
		if (m_depthTexture.isValid()) m_depthTexture.destroy();
		if (m_outputTexture.isValid()) m_outputTexture.destroy();
		*/
	}

	void ForwardPlus::beginFrameImGUI() {
		m_renderer.newImGuiFrame();
	}

	void ForwardPlus::draw(Scene* pScene) {
		
		if (pScene)
			updateLights(pScene);


		RenderContext context = m_pWindow->beginFrame();
		if (!context)
			return;

		if (pScene != nullptr) {

			collectMeshes(pScene);

			context.updateDescriptorSet(m_sceneDepthDescriptorSet, 0, m_objectBuffer.getCurrentBuffer());


			context.updateDescriptorSet(m_sceneDescriptorSet, 0, m_objectBuffer);
			context.updateDescriptorSet(m_sceneDescriptorSet, 1, m_lightBuffer.getCurrentBuffer());
			context.updateDescriptorSet(m_sceneDescriptorSet, 2, m_materialBuffer);
			context.updateDescriptorSet(m_sceneDescriptorSet, 3, m_materialIndicesBuffer);

			context.updateDescriptorSet(m_sceneDescriptorSet, 6, m_textures);

			context.updateDescriptorSet(m_lightCullingDescriptorSet, 2, m_lightBuffer);
			
			context.bindVertexBuffers(0, { m_vertexBuffer });
			context.bindIndexBuffer(m_indexBuffer);

			for (const auto& camera : pScene->getActiveCameras()) {
				SA_PROFILE_SCOPE("Draw camera");
				
				Matrix4x4 projViewMat = camera->getProjectionMatrix() * camera->getViewMatrix();

				PerFrameBuffer perFrame;
				perFrame.projViewMatrix = projViewMat;
				perFrame.viewPos = camera->getPosition();

				// Depth prepass
				context.beginRenderProgram(m_depthPreRenderProgram, m_depthPreFramebuffer, SubpassContents::DIRECT);
				context.bindPipeline(m_depthPrePipeline);
				context.setViewport(camera->getViewport());
				context.bindDescriptorSet(m_sceneDepthDescriptorSet, m_depthPrePipeline);


				if (m_indirectIndexedBuffer.getElementCount<DrawIndexedIndirectCommand>() > 0) {
					context.pushConstant(m_depthPrePipeline, ShaderStageFlagBits::VERTEX, perFrame);
					context.drawIndexedIndirect(m_indirectIndexedBuffer.getCurrentBuffer(), 0, m_indirectIndexedBuffer.getElementCount<DrawIndexedIndirectCommand>(), sizeof(DrawIndexedIndirectCommand));
				}

				context.endRenderProgram(m_depthPreRenderProgram);
				
				// Light culling
				context.bindPipeline(m_lightCullingPipeline);
				context.bindDescriptorSet(m_lightCullingDescriptorSet, m_lightCullingPipeline);

				context.pushConstant(m_lightCullingPipeline, ShaderStageFlagBits::COMPUTE, camera->getProjectionMatrix());
				context.pushConstant(m_lightCullingPipeline, ShaderStageFlagBits::COMPUTE, camera->getViewMatrix(), sizeof(Matrix4x4));

				context.dispatch(m_tileCount.x, m_tileCount.y, 1);

				// Main color pass
				context.beginRenderProgram(m_colorRenderProgram, m_colorFramebuffer, SubpassContents::DIRECT);
				context.bindPipeline(m_colorPipeline);
				context.bindDescriptorSet(m_sceneDescriptorSet, m_colorPipeline);

				context.setViewport(camera->getViewport());
					
				if (m_indirectIndexedBuffer.getElementCount<DrawIndexedIndirectCommand>() > 0) {
					context.pushConstant(m_colorPipeline, ShaderStageFlagBits::VERTEX, perFrame);
					context.drawIndexedIndirect(m_indirectIndexedBuffer.getCurrentBuffer(), 0, m_indirectIndexedBuffer.getElementCount<DrawIndexedIndirectCommand>(), sizeof(DrawIndexedIndirectCommand));
				}

				context.endRenderProgram(m_colorRenderProgram);
			}
			m_indirectIndexedBuffer.manualIncrement();
			

		}
		
		context.beginRenderProgram(m_debugLightHeatmapRenderProgram, m_debugLightHeatmapFramebuffer, SubpassContents::DIRECT);
		context.bindPipeline(m_debugLightHeatmapPipeline);
		context.bindDescriptorSet(m_debugLightHeatmapDescriptorSet, m_debugLightHeatmapPipeline);
		context.draw(6, 1);
		context.endRenderProgram(m_debugLightHeatmapRenderProgram);

		
		context.beginRenderProgram(m_composeRenderProgram, m_composeFramebuffer, SubpassContents::DIRECT);
			
		context.bindPipeline(m_composePipeline);
		context.bindDescriptorSet(m_composeDescriptorSet, m_composePipeline);
		context.draw(6, 1);

		context.endRenderProgram(m_composeRenderProgram);

		if (m_useImGui) {
			drawImGui(context);
		}

		m_pWindow->display();
	}

	const Texture& ForwardPlus::getOutputTexture() const {
		return m_outputTexture;
	}

	void ForwardPlus::updateLights(Scene* pScene) {
		SA_PROFILE_FUNCTION();
		
		m_lights.clear();
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
		if (value) {
			m_renderer.updateDescriptorSet(m_composeDescriptorSet, 1, m_debugLightHeatmap, m_nearestSampler);
		}
		else {
			m_renderer.updateDescriptorSet(m_composeDescriptorSet, 1, *AssetManager::get().loadDefaultBlackTexture(), m_nearestSampler);
		}
	}


}