#include "pch.h"
#include "ForwardPlus.h"

namespace sa {
	void ForwardPlus::createPreDepthPass(Extent extent) {
		if (m_sceneDepthDescriptorSet != NULL_RESOURCE) {
			m_renderer.freeDescriptorSet(m_sceneDepthDescriptorSet);
			m_sceneDepthDescriptorSet = NULL_RESOURCE;
		}
		if (m_depthPrePipeline != NULL_RESOURCE) {
			m_renderer.destroyPipeline(m_depthPrePipeline);
			m_depthPrePipeline = NULL_RESOURCE;
		}

		if (m_depthPreFramebuffer != NULL_RESOURCE) {
			m_renderer.destroyFramebuffer(m_depthPreFramebuffer);
			m_depthPreFramebuffer = NULL_RESOURCE;
		}

		if (m_depthPreRenderProgram != NULL_RESOURCE) {
			m_renderer.destroyRenderProgram(m_depthPreRenderProgram);
			m_depthPreRenderProgram = NULL_RESOURCE;
		}

		if (m_depthTexture.isValid()) 
			m_depthTexture.destroy();
		
		m_depthTexture = m_renderer.createTexture2D(sa::TextureTypeFlagBits::DEPTH_ATTACHMENT | sa::TextureTypeFlagBits::SAMPLED, extent);
		
		m_depthPreRenderProgram = m_renderer.createRenderProgram()
			.addDepthAttachment(m_depthTexture, true)
			.beginSubpass()
			.addAttachmentReference(0, SubpassAttachmentUsage::DepthTarget)
			.endSubpass()
			.end();

		m_depthPreFramebuffer = m_renderer.createFramebuffer(m_depthPreRenderProgram, { m_depthTexture });

		
		PipelineSettings settings = {};
		settings.dynamicStates.push_back(DynamicState::VIEWPORT);

		m_depthPrePipeline = m_renderer.createGraphicsPipeline(m_depthPreRenderProgram, 0, extent,
			"../Engine/shaders/ForwardPlusColorPass.vert.spv", settings);

		m_sceneDepthDescriptorSet = m_renderer.allocateDescriptorSet(m_depthPrePipeline, 0);

	}
	
	void ForwardPlus::createLightCullingShader() {
		if (m_lightCullingDescriptorSet != NULL_RESOURCE) {
			m_renderer.destroyPipeline(m_lightCullingDescriptorSet);
			m_lightCullingDescriptorSet = NULL_RESOURCE;
		}

		if (m_lightCullingPipeline != NULL_RESOURCE) {
			m_renderer.destroyPipeline(m_lightCullingPipeline);
			m_lightCullingPipeline = NULL_RESOURCE;
		}

		m_lightCullingPipeline = m_renderer.createComputePipeline("../Engine/shaders/LightCulling2.comp.spv");
		m_lightCullingDescriptorSet = m_renderer.allocateDescriptorSet(m_lightCullingPipeline, 0);


	}

	void ForwardPlus::createColorPass(Extent extent) {
		if (m_sceneDescriptorSet != NULL_RESOURCE) {
			m_renderer.freeDescriptorSet(m_sceneDescriptorSet);
			m_sceneDescriptorSet = NULL_RESOURCE;
		}

		if (m_colorPipeline != NULL_RESOURCE) {
			m_renderer.destroyPipeline(m_colorPipeline);
			m_colorPipeline = NULL_RESOURCE;
		}
		
		if (m_colorFramebuffer != NULL_RESOURCE) {
			m_renderer.destroyFramebuffer(m_colorFramebuffer);
			m_colorFramebuffer = NULL_RESOURCE;
		}

		if (m_colorRenderProgram != NULL_RESOURCE) {
			m_renderer.destroyRenderProgram(m_colorRenderProgram);
			m_colorRenderProgram = NULL_RESOURCE;
		}

		if (m_colorTexture.isValid()) 
			m_colorTexture.destroy();
		

		m_colorTexture = m_renderer.createTexture2D(sa::TextureTypeFlagBits::COLOR_ATTACHMENT | sa::TextureTypeFlagBits::SAMPLED, extent);
		drawData.colorTexture = m_colorTexture;

		m_colorRenderProgram = m_renderer.createRenderProgram()
			.addColorAttachment(true, m_colorTexture)
			.addDepthAttachment(m_depthTexture)
			.beginSubpass()
			.addAttachmentReference(0, sa::SubpassAttachmentUsage::ColorTarget)
			.addAttachmentReference(1, sa::SubpassAttachmentUsage::DepthTarget)
			.endSubpass()
			.end();
		m_renderer.setClearColor(m_colorRenderProgram, Color{ 0.0f, 0.0f, 0.1f });
	
		m_colorFramebuffer = m_renderer.createFramebuffer(m_colorRenderProgram, { m_colorTexture, m_depthTexture });
	
		PipelineSettings settings = {};
		settings.dynamicStates.push_back(DynamicState::VIEWPORT);

		m_colorPipeline = m_renderer.createGraphicsPipeline(m_colorRenderProgram, 0, extent,
			"../Engine/shaders/ForwardPlusColorPass.vert.spv", "../Engine/shaders/ForwardPlusColorPass.frag.spv", settings);

		
		m_sceneDescriptorSet = m_renderer.allocateDescriptorSet(m_colorPipeline, SET_PER_FRAME);


	}

	void ForwardPlus::resizeLightIndexBuffer(Extent extent) {
		m_tileCount = extent;
		m_tileCount += m_tileCount % TILE_SIZE;
		m_tileCount /= TILE_SIZE;

		size_t totalTileCount = m_tileCount.x * m_tileCount.y;

		if (m_lightIndexBuffer.isValid()) m_lightIndexBuffer.destroy();
		m_lightIndexBuffer = m_renderer.createBuffer(BufferType::STORAGE, sizeof(uint32_t) * MAX_LIGHTS_PER_TILE * totalTileCount);
	}


	void ForwardPlus::updateDescriptorSets() {

		m_renderer.updateDescriptorSet(m_sceneDepthDescriptorSet, 0, m_objectBuffer);

		m_renderer.updateDescriptorSet(m_lightCullingDescriptorSet, 0, m_depthTexture, m_linearSampler);
		m_renderer.updateDescriptorSet(m_lightCullingDescriptorSet, 1, m_lightIndexBuffer);
		m_renderer.updateDescriptorSet(m_lightCullingDescriptorSet, 2, m_lightBuffer);
		
		m_renderer.updateDescriptorSet(m_sceneDescriptorSet, 0, m_objectBuffer);
		m_renderer.updateDescriptorSet(m_sceneDescriptorSet, 1, m_lightBuffer);
		m_renderer.updateDescriptorSet(m_sceneDescriptorSet, 2, m_materialBuffer);
		m_renderer.updateDescriptorSet(m_sceneDescriptorSet, 3, m_materialIndicesBuffer);
		m_renderer.updateDescriptorSet(m_sceneDescriptorSet, 4, m_lightIndexBuffer);
		m_renderer.updateDescriptorSet(m_sceneDescriptorSet, 5, m_linearSampler);
		
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

	void ForwardPlus::onWindowResize(Extent extent) {
		createPreDepthPass(extent);
		createColorPass(extent);

		resizeLightIndexBuffer(extent);

		updateDescriptorSets();

		//DEBUG
		if (m_debugLightHeatmap.isValid()) 
			m_debugLightHeatmap.destroy();
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

		setShowHeatmap(false);


	}

	void ForwardPlus::init(Extent extent) {
		
		createPreDepthPass(extent);
		createLightCullingShader();
		createColorPass(extent);

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

		resizeLightIndexBuffer(extent);

		updateDescriptorSets();


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

		setShowHeatmap(false);



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

	void ForwardPlus::updateData(RenderContext& context) {
		SA_PROFILE_FUNCTION();
		context.updateDescriptorSet(m_sceneDepthDescriptorSet, 0, m_objectBuffer.getCurrentBuffer());


		context.updateDescriptorSet(m_sceneDescriptorSet, 0, m_objectBuffer);
		context.updateDescriptorSet(m_sceneDescriptorSet, 1, m_lightBuffer.getCurrentBuffer());
		context.updateDescriptorSet(m_sceneDescriptorSet, 2, m_materialBuffer);
		context.updateDescriptorSet(m_sceneDescriptorSet, 3, m_materialIndicesBuffer);
		context.updateDescriptorSet(m_sceneDescriptorSet, 4, m_lightIndexBuffer);

		context.updateDescriptorSet(m_sceneDescriptorSet, 6, m_textures);

		context.updateDescriptorSet(m_lightCullingDescriptorSet, 2, m_lightBuffer);

	}

	bool ForwardPlus::prepareRender(RenderContext& context, SceneCamera* pCamera) {
		if (!pCamera)
			return false;
		

		context.bindVertexBuffers(0, { m_vertexBuffer.getCurrentBuffer()});
		context.bindIndexBuffer(m_indexBuffer.getCurrentBuffer());

		Matrix4x4 projViewMat = pCamera->getProjectionMatrix() * pCamera->getViewMatrix();

		PerFrameBuffer perFrame;
		perFrame.projViewMatrix = projViewMat;
		perFrame.viewPos = pCamera->getPosition();

		// Depth prepass
		context.beginRenderProgram(m_depthPreRenderProgram, m_depthPreFramebuffer, SubpassContents::DIRECT);
		context.bindPipeline(m_depthPrePipeline);
		context.setViewport(pCamera->getViewport());
		context.bindDescriptorSet(m_sceneDepthDescriptorSet, m_depthPrePipeline);


		if (m_indirectIndexedBuffer.getElementCount<DrawIndexedIndirectCommand>() > 0) {
			context.pushConstant(m_depthPrePipeline, ShaderStageFlagBits::VERTEX, perFrame);
			context.drawIndexedIndirect(m_indirectIndexedBuffer.getCurrentBuffer(), 0, m_indirectIndexedBuffer.getElementCount<DrawIndexedIndirectCommand>(), sizeof(DrawIndexedIndirectCommand));
		}

		context.endRenderProgram(m_depthPreRenderProgram);

		// Light culling
		context.bindPipeline(m_lightCullingPipeline);
		context.bindDescriptorSet(m_lightCullingDescriptorSet, m_lightCullingPipeline);

		context.pushConstant(m_lightCullingPipeline, ShaderStageFlagBits::COMPUTE, pCamera->getProjectionMatrix());
		context.pushConstant(m_lightCullingPipeline, ShaderStageFlagBits::COMPUTE, pCamera->getViewMatrix(), sizeof(Matrix4x4));

		context.dispatch(m_tileCount.x, m_tileCount.y, 1);

		// TODO generate shadowMaps


		return true;
	}

	void ForwardPlus::render(RenderContext& context, SceneCamera* pCamera, ResourceID framebuffer) {
		if (!pCamera)
			return;

		Matrix4x4 projViewMat = pCamera->getProjectionMatrix() * pCamera->getViewMatrix();

		PerFrameBuffer perFrame;
		perFrame.projViewMatrix = projViewMat;
		perFrame.viewPos = pCamera->getPosition();

		// Main color pass
		context.beginRenderProgram(m_colorRenderProgram, framebuffer, SubpassContents::DIRECT);
		context.bindPipeline(m_colorPipeline);
		context.bindDescriptorSet(m_sceneDescriptorSet, m_colorPipeline);

		context.setViewport(pCamera->getViewport());

		if (m_indirectIndexedBuffer.getElementCount<DrawIndexedIndirectCommand>() > 0) {
			context.pushConstant(m_colorPipeline, ShaderStageFlagBits::VERTEX | ShaderStageFlagBits::FRAGMENT, perFrame);
			context.pushConstant(m_colorPipeline, ShaderStageFlagBits::FRAGMENT, m_tileCount.x, sizeof(perFrame));

			context.drawIndexedIndirect(m_indirectIndexedBuffer.getCurrentBuffer(), 0, m_indirectIndexedBuffer.getElementCount<DrawIndexedIndirectCommand>(), sizeof(DrawIndexedIndirectCommand));
		}

		context.endRenderProgram(m_colorRenderProgram);

		
		context.beginRenderProgram(m_debugLightHeatmapRenderProgram, m_debugLightHeatmapFramebuffer, SubpassContents::DIRECT);
		context.bindPipeline(m_debugLightHeatmapPipeline);
		context.bindDescriptorSet(m_debugLightHeatmapDescriptorSet, m_debugLightHeatmapPipeline);
		context.pushConstant(m_debugLightHeatmapPipeline, ShaderStageFlagBits::FRAGMENT, m_tileCount.x);
		context.draw(6, 1);
		context.endRenderProgram(m_debugLightHeatmapRenderProgram);

	}

	void ForwardPlus::endRender(RenderContext& context) {
		m_vertexBuffer.manualIncrement();
		m_indexBuffer.manualIncrement();
		m_indirectIndexedBuffer.manualIncrement();
	}

	ResourceID ForwardPlus::createColorFramebuffer(const Texture2D& outputTexture) {
		return m_renderer.createFramebuffer(m_colorRenderProgram, { outputTexture, m_depthTexture });
	}

	void ForwardPlus::updateLights(Scene* pScene) {
		SA_PROFILE_FUNCTION();
		
		m_lights.clear();
		pScene->view<comp::Transform, comp::Light>().each([](const comp::Transform& transform, comp::Light& light) {
			light.values.position = transform.position;
		});
		
		pScene->view<comp::Light>().each([&](comp::Light& light) {
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