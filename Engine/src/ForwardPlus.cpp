#include "pch.h"
#include "Graphics/RenderTechniques/ForwardPlus.h"
#include "Graphics\RenderLayers\ShadowRenderLayer.h"

#include "Engine.h"

#include "Graphics\DebugRenderer.h"

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

		auto code = ReadSPVFile((Engine::GetShaderDirectory() / "LightCulling.comp.spv").generic_string().c_str());
		m_lightCullingLayout.createFromShaders({ code });
		m_lightCullingShader.create(code, ShaderStageFlagBits::COMPUTE);
		m_lightCullingPipeline = m_renderer.createComputePipeline(m_lightCullingShader, m_lightCullingLayout);
		
	}

	void ForwardPlus::createColorPass() {
		
		if (m_colorRenderProgram != NULL_RESOURCE) {
			m_renderer.destroyRenderProgram(m_colorRenderProgram);
			m_colorRenderProgram = NULL_RESOURCE;
		}

		Format colorFormat = m_renderer.selectFormat({ Format::R32G32B32A32_SFLOAT }, TextureUsageFlagBits::COLOR_ATTACHMENT | TextureUsageFlagBits::SAMPLED);

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

	void ForwardPlus::initializeMainRenderData(ForwardPlusRenderData& data, Extent extent)
	{
		Format colorFormat = m_renderer.getAttachmentFormat(m_colorRenderProgram, 0);
		Format depthFormat = m_renderer.getAttachmentFormat(m_colorRenderProgram, 1);


		data.colorTexture.create2D(TextureUsageFlagBits::COLOR_ATTACHMENT | TextureUsageFlagBits::SAMPLED, extent, colorFormat);
		data.depthTexture.create2D(TextureUsageFlagBits::DEPTH_ATTACHMENT | TextureUsageFlagBits::SAMPLED, extent, depthFormat);


		//Depth pre pass
		data.depthFramebuffer = m_renderer.createFramebuffer(m_depthPreRenderProgram, &data.depthTexture, 1);

		// Light culling pass
		data.tileCount = { extent.width, extent.height };
		data.tileCount += (TILE_SIZE - data.tileCount % TILE_SIZE);
		data.tileCount /= TILE_SIZE;

		size_t totalTileCount = data.tileCount.x * data.tileCount.y;
		if (data.lightCullingDescriptorSet == NULL_RESOURCE)
			data.lightCullingDescriptorSet = m_lightCullingLayout.allocateDescriptorSet(0);

		data.lightIndexBuffer.create(BufferType::STORAGE, sizeof(uint32_t) * MAX_LIGHTS_PER_TILE * totalTileCount);

		// Color pass
		const DynamicTexture textures[] = { data.colorTexture, data.depthTexture };
		data.colorFramebuffer = m_renderer.createFramebuffer(m_colorRenderProgram, textures, 2);


		m_renderer.updateDescriptorSet(data.lightCullingDescriptorSet, 0, data.depthTexture, m_linearSampler);	// read depth texture
		m_renderer.updateDescriptorSet(data.lightCullingDescriptorSet, 1, data.lightIndexBuffer);		// write what lights are in what tiles

		// ----------- DEBUG -------------------

		data.debugLightHeatmap.create2D(TextureUsageFlagBits::COLOR_ATTACHMENT | TextureUsageFlagBits::SAMPLED, { data.tileCount.x, data.tileCount.y }, m_debugTextureFormat);
		if(data.debugLightHeatmapDescriptorSet == NULL_RESOURCE) {
			data.debugLightHeatmapDescriptorSet = m_debugHeatmapLayout.allocateDescriptorSet(0);
		}
		m_renderer.updateDescriptorSet(data.debugLightHeatmapDescriptorSet, 0, data.lightIndexBuffer);

		data.debugLightHeatmapFramebuffer = m_renderer.createFramebuffer(m_debugLightHeatmapRenderProgram, &data.debugLightHeatmap, 1);
		// ----------------------------------

		data.isInitialized = true;

	}

	void ForwardPlus::cleanupMainRenderData(ForwardPlusRenderData& data) {
		if (data.colorTexture.isValid())
			data.colorTexture.destroy();
		if (data.depthTexture.isValid())
			data.depthTexture.destroy();

		if (data.depthFramebuffer != NULL_RESOURCE) {
			m_renderer.destroyFramebuffer(data.depthFramebuffer);
			data.depthFramebuffer = NULL_RESOURCE;
		}

		if (data.lightIndexBuffer.isValid())
			data.lightIndexBuffer.destroy();

		if (data.colorFramebuffer != NULL_RESOURCE) {
			m_renderer.destroyFramebuffer(data.colorFramebuffer);
			data.colorFramebuffer = NULL_RESOURCE;
		}

		//DEBUG 
		if (data.debugLightHeatmap.isValid())
			data.debugLightHeatmap.destroy();

		if (data.debugLightHeatmapFramebuffer != NULL_RESOURCE) {
			m_renderer.destroyFramebuffer(data.debugLightHeatmapFramebuffer);
			data.debugLightHeatmapFramebuffer = NULL_RESOURCE;
		}
		
	}

	ForwardPlus::ForwardPlus(ShadowRenderLayer* pShadowRenderLayer) : IRenderLayer() {
		m_pShadowRenderLayer = pShadowRenderLayer;
	}

	void ForwardPlus::onRenderTargetResize(UUID renderTargetID, Extent oldExtent, Extent newExtent) {
		ForwardPlusRenderData& renderTargetData = getRenderTargetData(renderTargetID);
		cleanupMainRenderData(renderTargetData);
		initializeMainRenderData(renderTargetData, newExtent);
		SA_DEBUG_LOG_INFO("Initialized Forward Plus data for RenderTarget UUID: ", renderTargetID, " with extent { w:", newExtent.width, ", h:", newExtent.height, " }");

	}


	void ForwardPlus::init() {
		if (m_isInitialized)
			return;
		createPreDepthPass();
		createLightCullingShader();
		createColorPass();

		// Samplers
		
		m_linearSampler = m_renderer.createSampler(FilterMode::LINEAR);
		m_nearestSampler = m_renderer.createSampler(FilterMode::NEAREST);


		//DEBUG
		
		m_debugHeatmapVertexShader.create(sa::ReadSPVFile((Engine::GetShaderDirectory() / "DebugHeatmap.vert.spv").generic_string().c_str()), ShaderStageFlagBits::VERTEX);
		m_debugHeatmapFragmentShader.create(sa::ReadSPVFile((Engine::GetShaderDirectory() / "DebugHeatmap.frag.spv").generic_string().c_str()), ShaderStageFlagBits::FRAGMENT);
		m_debugHeatmapLayout.createFromShaders({ m_debugHeatmapVertexShader, m_debugHeatmapFragmentShader });

		m_debugTextureFormat = sa::Format::R8G8B8A8_UNORM;
		m_debugLightHeatmapRenderProgram = m_renderer.createRenderProgram()
			.addColorAttachment(AttachmentFlagBits::eClear | AttachmentFlagBits::eSampled | AttachmentFlagBits::eStore, m_debugTextureFormat)
			.beginSubpass()
			.addAttachmentReference(0, SubpassAttachmentUsage::ColorTarget)
			.endSubpass()
			.end();

		sa::PipelineSettings settings = {};
		settings.dynamicStates.push_back(sa::DynamicState::VIEWPORT);
		settings.dynamicStates.push_back(sa::DynamicState::SCISSOR);
		std::array<sa::Shader, 2> shaders = { m_debugHeatmapVertexShader, m_debugHeatmapFragmentShader };
		m_debugLightHeatmapPipeline = m_renderer.createGraphicsPipeline(
			m_debugHeatmapLayout,
			shaders.data(),
			shaders.size(),
			m_debugLightHeatmapRenderProgram,
			0,
			{ 0, 0 },
			settings
		);

		m_isInitialized = true;
	}

	void ForwardPlus::cleanup() {
		m_lightCullingShader.destroy();

		m_debugHeatmapLayout.destroy();
		m_debugHeatmapVertexShader.destroy();
		m_debugHeatmapFragmentShader.destroy();
		m_renderer.destroyPipeline(m_debugLightHeatmapPipeline);
		m_renderer.destroyRenderProgram(m_debugLightHeatmapRenderProgram);
	}

	bool ForwardPlus::render(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sc) {
		SA_PROFILE_FUNCTION();
		if (!pCamera)
			return false;
		ForwardPlusRenderData& data = getRenderTargetData(pRenderTarget->getID());
		if (!data.isInitialized) {
			cleanupMainRenderData(data);
			initializeMainRenderData(data, pRenderTarget->getExtent());
			SA_DEBUG_LOG_INFO("Initialized Forward Plus data for RenderTarget UUID: ", pRenderTarget->getID(), " with extent { w:", pRenderTarget->getExtent().width, ", h:", pRenderTarget->getExtent().height, " }");
		}
		Rectf cameraViewport = pCamera->getViewport();
		Rect viewport = {
			{
				static_cast<int32_t>(cameraViewport.offset.x * pRenderTarget->getExtent().width),
				static_cast<int32_t>(cameraViewport.offset.y * pRenderTarget->getExtent().height)
			},
			{
				static_cast<uint32_t>(cameraViewport.extent.x * pRenderTarget->getExtent().width),
				static_cast<uint32_t>(cameraViewport.extent.y * pRenderTarget->getExtent().height)
			}
		};
		if ((viewport.extent.height & viewport.extent.width) == 0) {
			return false;
		}
		//pCamera->setAspectRatio((float)viewport.extent.width / viewport.extent.height);
		
		PerFrameBuffer perFrame;
		perFrame.projMat = pCamera->getProjectionMatrix();
		perFrame.viewMat = pCamera->getViewMatrix();
		perFrame.viewPos = glm::vec4(pCamera->getPosition(), 1.0f);

		context.beginRenderProgram(m_depthPreRenderProgram, data.depthFramebuffer, SubpassContents::DIRECT);
		for (auto& collection : sc) {
			if (!collection.readyDescriptorSets(context)) {
				continue;
			}
			
			collection.recreatePipelines(m_colorRenderProgram, m_depthPreRenderProgram, pRenderTarget->getExtent());

			// Depth prepass
			
			collection.bindDepthPipeline(context);

			context.bindVertexBuffers(0, &collection.getVertexBuffer(), 1);
			context.bindIndexBuffer(collection.getIndexBuffer());

			context.setViewport(viewport);
			context.setScissor(viewport);
			context.setDepthBiasEnable(false);
			context.setDepthBias(0.0f, 0.0f, 0.0f);
			context.setCullMode(sa::CullModeFlagBits::BACK);

			context.bindDescriptorSet(collection.getSceneDescriptorSetDepthPass());


			if (collection.getDrawCommandBuffer().getElementCount<DrawIndexedIndirectCommand>() > 0) {
				context.pushConstant(ShaderStageFlagBits::VERTEX, perFrame);
				size_t drawCallCount = collection.getDrawCommandBuffer().getElementCount<DrawIndexedIndirectCommand>();
				context.drawIndexedIndirect(collection.getDrawCommandBuffer(), 0, drawCallCount, sizeof(DrawIndexedIndirectCommand));
				Engine::GetEngineStatistics().drawCalls += drawCallCount;
			}
		}

		context.endRenderProgram(m_depthPreRenderProgram);

		context.updateDescriptorSet(data.lightCullingDescriptorSet, 1, data.lightIndexBuffer.getBuffer());
		context.updateDescriptorSet(data.lightCullingDescriptorSet, 2, sc.getLightBuffer());

		// Light culling
		context.bindPipelineLayout(m_lightCullingLayout);
		context.bindPipeline(m_lightCullingPipeline);
		context.bindDescriptorSet(data.lightCullingDescriptorSet);

		context.pushConstant(ShaderStageFlagBits::COMPUTE, pCamera->getProjectionMatrix());
		context.pushConstant(ShaderStageFlagBits::COMPUTE, pCamera->getViewMatrix(), sizeof(Matrix4x4));

		context.dispatch(data.tileCount.x, data.tileCount.y, 1);
		Engine::GetEngineStatistics().dispatchCalls++;

		// Main color pass
		context.beginRenderProgram(m_colorRenderProgram, data.colorFramebuffer, SubpassContents::DIRECT);
		for (auto& collection : sc) {
			if (!collection.readyDescriptorSets(context)) {
				continue;
			}
			collection.bindColorPipeline(context);

			context.updateDescriptorSet(collection.getSceneDescriptorSetColorPass(), 1, sc.getLightBuffer());
			context.updateDescriptorSet(collection.getSceneDescriptorSetColorPass(), 4, data.lightIndexBuffer.getBuffer());
			if (m_pShadowRenderLayer && m_pShadowRenderLayer->isActive()) {
				context.updateDescriptorSet(collection.getSceneDescriptorSetColorPass(), 5, sc.getShadowDataBuffer());

				context.updateDescriptorSet(collection.getSceneDescriptorSetColorPass(), 7, m_pShadowRenderLayer->getPreferencesBuffer());
				context.updateDescriptorSet(collection.getSceneDescriptorSetColorPass(), 
					8,
					sc.getShadowTextures().data(), 
					sc.getShadowTextureCount(), 
					m_pShadowRenderLayer->getShadowSampler(), 
					0
				);
				context.updateDescriptorSet(collection.getSceneDescriptorSetColorPass(),
					9,
					sc.getShadowCubeTextures().data(),
					sc.getShadowCubeTextureCount(),
					m_pShadowRenderLayer->getShadowSampler(),
					0
				);

			}
			else {
				if (!m_defaultShadowPreferencesBuffer.isValid()) {
					uint32_t shadowsEnabled = false;
					m_defaultShadowPreferencesBuffer.create(BufferType::UNIFORM, sizeof(shadowsEnabled), &shadowsEnabled);
				}

				if (!m_defaultShadowDataBuffer.isValid()) {
					m_defaultShadowDataBuffer.create(BufferType::STORAGE);
				}
				context.updateDescriptorSet(collection.getSceneDescriptorSetColorPass(), 5, m_defaultShadowDataBuffer);
				context.updateDescriptorSet(collection.getSceneDescriptorSetColorPass(), 7, m_defaultShadowPreferencesBuffer);
					
			}
			context.updateDescriptorSet(collection.getSceneDescriptorSetColorPass(), 6, m_linearSampler);
			

			context.bindDescriptorSet(collection.getSceneDescriptorSetColorPass());

			context.setViewport(viewport);

			if (collection.getDrawCommandBuffer().getElementCount<DrawIndexedIndirectCommand>() > 0) {
				context.pushConstant(ShaderStageFlagBits::VERTEX | ShaderStageFlagBits::FRAGMENT, perFrame);
				context.pushConstant(ShaderStageFlagBits::FRAGMENT, data.tileCount.x, sizeof(perFrame));
				size_t drawCallCount = collection.getDrawCommandBuffer().getElementCount<DrawIndexedIndirectCommand>();
				context.drawIndexedIndirect(collection.getDrawCommandBuffer(), 0, drawCallCount, sizeof(DrawIndexedIndirectCommand));
				Engine::GetEngineStatistics().drawCalls += drawCallCount;
			}
		}
		
		//Finally render debug stuff
		if (!DebugRenderer::Get().isInitialized())
			DebugRenderer::Get().initialize(m_colorRenderProgram);

		DebugRenderer::Get().render(context, viewport.extent, *pCamera);


		context.endRenderProgram(m_colorRenderProgram);

		
		if(data.renderDebugHeatmap) {
			context.beginRenderProgram(m_debugLightHeatmapRenderProgram, data.debugLightHeatmapFramebuffer, SubpassContents::DIRECT);
			context.bindPipelineLayout(m_debugHeatmapLayout);
			context.bindPipeline(m_debugLightHeatmapPipeline);
			Rect vp = {};
			vp.offset = { 0, 0 };
			vp.extent = { data.tileCount.x, data.tileCount.y };
			context.setViewport(vp);
			context.setScissor(vp);
			context.bindDescriptorSet(data.debugLightHeatmapDescriptorSet);
			context.pushConstant(ShaderStageFlagBits::FRAGMENT, data.tileCount.x);
			context.draw(6, 1);
			context.endRenderProgram(m_debugLightHeatmapRenderProgram);

			Engine::GetEngineStatistics().drawCalls++;

			data.debugLightHeatmap.sync(context);
		}
		
		data.depthTexture.sync(context);
		data.colorTexture.sync(context);

		context.syncFramebuffer(data.colorFramebuffer);
		context.syncFramebuffer(data.depthFramebuffer);
		context.syncFramebuffer(data.debugLightHeatmapFramebuffer);

		pRenderTarget->setOutputTexture(data.colorTexture);
		return true;
	}

	bool ForwardPlus::postRender(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget,
		SceneCollection& sceneCollection)
	{
		ForwardPlusRenderData& data = getRenderTargetData(pRenderTarget->getID());
		if (data.isInitialized) {
			//data.colorTexture.swap();
			//data.depthTexture.swap();
			data.lightIndexBuffer.swap();

			//m_renderer.swapFramebuffer(data.colorFramebuffer);
			//m_renderer.swapFramebuffer(data.depthFramebuffer);

			//data.debugLightHeatmap.swap();
			//m_renderer.swapFramebuffer(data.debugLightHeatmapFramebuffer);
		}
		return true;
	}
}
