#include "pch.h"
#include "Graphics/RenderTechniques/ForwardPlus.h"
#include "Graphics\RenderLayers\ShadowRenderLayer.h"

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

		auto code = ReadSPVFile((Engine::getShaderDirectory() / "LightCulling.comp.spv").generic_string().c_str());
		m_lightCullingLayout.createFromShaders({ code });
		m_lightCullingShader.create({ code }, ShaderStageFlagBits::COMPUTE);
		m_lightCullingPipeline = m_renderer.createComputePipeline(m_lightCullingShader, m_lightCullingLayout);
		
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

	void ForwardPlus::initializeMainRenderData(ForwardPlusRenderData& data, Extent extent)
	{
		Format colorFormat = m_renderer.getAttachmentFormat(m_colorRenderProgram, 0);
		Format depthFormat = m_renderer.getAttachmentFormat(m_colorRenderProgram, 1);


		data.colorTexture = DynamicTexture2D(TextureTypeFlagBits::COLOR_ATTACHMENT | TextureTypeFlagBits::SAMPLED, extent, colorFormat);
		data.depthTexture = DynamicTexture2D(TextureTypeFlagBits::DEPTH_ATTACHMENT | TextureTypeFlagBits::SAMPLED, extent, depthFormat);


		//Depth pre pass
		data.depthFramebuffer = m_renderer.createFramebuffer(m_depthPreRenderProgram, { (DynamicTexture)data.depthTexture });

		// Light culling pass
		data.tileCount = { extent.width, extent.height };
		data.tileCount += (TILE_SIZE - data.tileCount % TILE_SIZE);
		data.tileCount /= TILE_SIZE;

		size_t totalTileCount = data.tileCount.x * data.tileCount.y;
		if (data.lightCullingDescriptorSet == NULL_RESOURCE)
			data.lightCullingDescriptorSet = m_lightCullingLayout.allocateDescriptorSet(0);

		data.lightIndexBuffer = m_renderer.createDynamicBuffer(BufferType::STORAGE, sizeof(uint32_t) * MAX_LIGHTS_PER_TILE * totalTileCount);

		// Color pass
		data.colorFramebuffer = m_renderer.createFramebuffer(m_colorRenderProgram, { (DynamicTexture)data.colorTexture, data.depthTexture });


		m_renderer.updateDescriptorSet(data.lightCullingDescriptorSet, 0, data.depthTexture, m_linearSampler);	// read depth texture
		m_renderer.updateDescriptorSet(data.lightCullingDescriptorSet, 1, data.lightIndexBuffer);		// write what lights are in what tiles

		// ----------- DEBUG -------------------
		data.debugLightHeatmap = DynamicTexture2D(TextureTypeFlagBits::COLOR_ATTACHMENT | TextureTypeFlagBits::SAMPLED, { data.tileCount.x, data.tileCount.y });
		data.debugLightHeatmapRenderProgram = m_renderer.createRenderProgram()
			.addColorAttachment(AttachmentFlagBits::eClear | AttachmentFlagBits::eSampled | AttachmentFlagBits::eStore, data.debugLightHeatmap)
			.beginSubpass()
			.addAttachmentReference(0, SubpassAttachmentUsage::ColorTarget)
			.endSubpass()
			.end();


		data.debugLightHeatmapFramebuffer = m_renderer.createFramebuffer(data.debugLightHeatmapRenderProgram, { data.debugLightHeatmap });
		std::array<sa::Shader, 2> shaders = { m_debugHeatmapVertexShader, m_debugHeatmapFragmentShader };
		data.debugLightHeatmapPipeline = m_renderer.createGraphicsPipeline(
			m_debugHeatmapLayout,
			shaders.data(),
			shaders.size(),
			data.debugLightHeatmapRenderProgram,
			0,
			{ data.tileCount.x, data.tileCount.y }
		);

		data.debugLightHeatmapDescriptorSet = m_debugHeatmapLayout.allocateDescriptorSet(0);
		m_renderer.updateDescriptorSet(data.debugLightHeatmapDescriptorSet, 0, data.lightIndexBuffer);
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

		if (data.debugLightHeatmapPipeline != NULL_RESOURCE) {
			m_renderer.destroyPipeline(data.debugLightHeatmapPipeline);
			data.debugLightHeatmapPipeline = NULL_RESOURCE;
		}

		if (data.debugLightHeatmapRenderProgram != NULL_RESOURCE) {
			m_renderer.destroyRenderProgram(data.debugLightHeatmapRenderProgram);
			data.debugLightHeatmapRenderProgram = NULL_RESOURCE;
		}
		
	}

	ForwardPlus::ForwardPlus(ShadowRenderLayer* pShadowRenderLayer) : IRenderLayer() {
		m_pShadowRenderLayer = pShadowRenderLayer;
	}

	void ForwardPlus::onRenderTargetResize(UUID renderTargetID, Extent oldExtent, Extent newExtent) {
		ForwardPlusRenderData& renderTargetData = getRenderTargetData(renderTargetID);
		cleanupMainRenderData(renderTargetData);
		initializeMainRenderData(renderTargetData, newExtent);
	}


	void ForwardPlus::init() {
		if (m_isInitialized)
			return;
		createPreDepthPass();
		createLightCullingShader();
		createColorPass();

		// Samplers
		SamplerInfo info = {};
		info.addressModeU = SamplerAddressMode::CLAMP_TO_BORDER;
		info.addressModeV = SamplerAddressMode::CLAMP_TO_BORDER;
		info.addressModeW = SamplerAddressMode::CLAMP_TO_BORDER;
		info.borderColor = sa::BorderColor::FLOAT_OPAQUE_WHITE;
		info.magFilter = FilterMode::LINEAR;
		info.minFilter = FilterMode::LINEAR;
		info.compareEnable = true;
		info.compareOp = CompareOp::GREATER;

		m_shadowSampler = m_renderer.createSampler(info);
		m_linearSampler = m_renderer.createSampler(FilterMode::LINEAR);
		m_nearestSampler = m_renderer.createSampler(FilterMode::NEAREST);


		//DEBUG
		
		m_debugHeatmapVertexShader.create(sa::ReadSPVFile((Engine::getShaderDirectory() / "DebugHeatmap.vert.spv").generic_string().c_str()), ShaderStageFlagBits::VERTEX);
		m_debugHeatmapFragmentShader.create(sa::ReadSPVFile((Engine::getShaderDirectory() / "DebugHeatmap.frag.spv").generic_string().c_str()), ShaderStageFlagBits::FRAGMENT);
		m_debugHeatmapLayout.createFromShaders({ m_debugHeatmapVertexShader, m_debugHeatmapFragmentShader });



		m_isInitialized = true;
	}

	void ForwardPlus::cleanup() {
		m_lightCullingShader.destroy();
		m_debugHeatmapVertexShader.destroy();
		m_debugHeatmapFragmentShader.destroy();
	}

	bool ForwardPlus::render(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sc) {
		SA_PROFILE_FUNCTION();
		if (!pCamera)
			return false;
		ForwardPlusRenderData& data = getRenderTargetData(pRenderTarget->getID());
		if (!data.isInitialized) {
			cleanupMainRenderData(data);
			initializeMainRenderData(data, pRenderTarget->getExtent());
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
		pCamera->setAspectRatio((float)viewport.extent.width / viewport.extent.height);
		Matrix4x4 projViewMat = pCamera->getProjectionMatrix() * pCamera->getViewMatrix();

		PerFrameBuffer perFrame;
		perFrame.projViewMatrix = projViewMat;
		perFrame.viewPos = pCamera->getPosition();

		context.beginRenderProgram(m_depthPreRenderProgram, data.depthFramebuffer, SubpassContents::DIRECT);
		for (auto& collection : sc) {
			if (!collection.readyDescriptorSets(context)) {
				continue;
			}
			
			collection.recreatePipelines(m_colorRenderProgram, m_depthPreRenderProgram, pRenderTarget->getExtent());

			// Depth prepass
			
			collection.bindDepthPipeline(context);

			context.bindVertexBuffers(0, { collection.getVertexBuffer() });
			context.bindIndexBuffer(collection.getIndexBuffer());

			context.setViewport(viewport);

			context.bindDescriptorSet(collection.getSceneDescriptorSetDepthPass());



			if (collection.getDrawCommandBuffer().getElementCount<DrawIndexedIndirectCommand>() > 0) {
				context.pushConstant(ShaderStageFlagBits::VERTEX, perFrame);
				context.drawIndexedIndirect(collection.getDrawCommandBuffer(), 0, collection.getDrawCommandBuffer().getElementCount<DrawIndexedIndirectCommand>(), sizeof(DrawIndexedIndirectCommand));
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
				context.updateDescriptorSet(collection.getSceneDescriptorSetColorPass(), 5, m_pShadowRenderLayer->getShadowDataBuffer());
				context.updateDescriptorSet(collection.getSceneDescriptorSetColorPass(), 7,
					m_pShadowRenderLayer->getShadowTextures().data(), m_pShadowRenderLayer->getShadowTextureCount(), m_shadowSampler, 0);
			}
			context.updateDescriptorSet(collection.getSceneDescriptorSetColorPass(), 6, m_linearSampler);
			

			context.bindDescriptorSet(collection.getSceneDescriptorSetColorPass());

			context.setViewport(viewport);

			if (collection.getDrawCommandBuffer().getElementCount<DrawIndexedIndirectCommand>() > 0) {
				context.pushConstant(ShaderStageFlagBits::VERTEX | ShaderStageFlagBits::FRAGMENT, perFrame);
				context.pushConstant(ShaderStageFlagBits::FRAGMENT, data.tileCount.x, sizeof(perFrame));

				context.drawIndexedIndirect(collection.getDrawCommandBuffer(), 0, collection.getDrawCommandBuffer().getElementCount<DrawIndexedIndirectCommand>(), sizeof(DrawIndexedIndirectCommand));
			}
		}

		context.endRenderProgram(m_colorRenderProgram);

		
		if(data.renderDebugHeatmap) {
			context.beginRenderProgram(data.debugLightHeatmapRenderProgram, data.debugLightHeatmapFramebuffer, SubpassContents::DIRECT);
			context.bindPipeline(data.debugLightHeatmapPipeline);
			context.bindDescriptorSet(data.debugLightHeatmapDescriptorSet);
			context.pushConstant(ShaderStageFlagBits::FRAGMENT, data.tileCount.x);
			context.draw(6, 1);
			context.endRenderProgram(data.debugLightHeatmapRenderProgram);
		}
		

		pRenderTarget->setOutputTexture(data.colorTexture);
		return true;
	}

	bool ForwardPlus::postRender(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget,
		SceneCollection& sceneCollection)
	{
		ForwardPlusRenderData& data = getRenderTargetData(pRenderTarget->getID());
		if (data.isInitialized) {
			data.colorTexture.swap();
			data.depthTexture.swap();
			data.lightIndexBuffer.swap();

			m_renderer.swapFramebuffer(data.colorFramebuffer);
			m_renderer.swapFramebuffer(data.depthFramebuffer);

			data.debugLightHeatmap.swap();
			m_renderer.swapFramebuffer(data.debugLightHeatmapFramebuffer);
		}
		return true;
	}
}
