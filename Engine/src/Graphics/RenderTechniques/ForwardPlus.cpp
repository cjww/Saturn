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


		m_renderer.updateDescriptorSet(data.lightCullingDescriptorSet, 0, data.depthTexture, m_linearSampler);	// read depth texture
		m_renderer.updateDescriptorSet(data.lightCullingDescriptorSet, 1, data.lightIndexBuffer);				// write what lights are in what tiles
		m_renderer.updateDescriptorSet(data.sceneDescriptorSet, 5, m_linearSampler);

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

	bool ForwardPlus::prepareRender(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sc) {
		if (!pCamera)
			return false;

		RenderTarget::MainRenderData& data = pRenderTarget->mainRenderData;
		
		if (!data.isInitialized) {
			pRenderTarget->cleanupMainRenderData();
			initializeMainRenderData(data, pRenderTarget->extent);
			data.isInitialized = true;			
		}


		context.updateDescriptorSet(data.sceneDepthDescriptorSet, 0, sc.getObjectBuffer());

		context.updateDescriptorSet(data.sceneDescriptorSet, 0, sc.getObjectBuffer());
		context.updateDescriptorSet(data.sceneDescriptorSet, 1, sc.getLightBuffer());
		context.updateDescriptorSet(data.sceneDescriptorSet, 2, sc.getMaterialBuffer());
		context.updateDescriptorSet(data.sceneDescriptorSet, 3, sc.getMaterialIndicesBuffer());
		context.updateDescriptorSet(data.sceneDescriptorSet, 4, data.lightIndexBuffer);
		context.updateDescriptorSet(data.sceneDescriptorSet, 6, sc.getTextures());

		context.updateDescriptorSet(data.lightCullingDescriptorSet, 1, data.lightIndexBuffer);
		context.updateDescriptorSet(data.lightCullingDescriptorSet, 2, sc.getLightBuffer());



		context.bindVertexBuffers(0, { sc.getVertexBuffer()});
		context.bindIndexBuffer(sc.getIndexBuffer());

		Matrix4x4 projViewMat = pCamera->getProjectionMatrix() * pCamera->getViewMatrix();

		PerFrameBuffer perFrame;
		perFrame.projViewMatrix = projViewMat;
		perFrame.viewPos = pCamera->getPosition();

		// Depth prepass
		context.beginRenderProgram(m_depthPreRenderProgram, data.depthFramebuffer, SubpassContents::DIRECT);
		context.bindPipeline(data.depthPipeline);
		
		context.setViewport(pCamera->getViewport());

		context.bindDescriptorSet(data.sceneDepthDescriptorSet, data.depthPipeline);


		if (sc.getDrawCommandBuffer().getElementCount<DrawIndexedIndirectCommand>() > 0) {
			context.pushConstant(data.depthPipeline, ShaderStageFlagBits::VERTEX, perFrame);
			context.drawIndexedIndirect(sc.getDrawCommandBuffer(), 0, sc.getDrawCommandBuffer().getElementCount<DrawIndexedIndirectCommand>(), sizeof(DrawIndexedIndirectCommand));
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

	void ForwardPlus::render(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sc) {
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

		if (sc.getDrawCommandBuffer().getElementCount<DrawIndexedIndirectCommand>() > 0) {
			context.pushConstant(data.colorPipeline, ShaderStageFlagBits::VERTEX | ShaderStageFlagBits::FRAGMENT, perFrame);
			context.pushConstant(data.colorPipeline, ShaderStageFlagBits::FRAGMENT, data.tileCount.x, sizeof(perFrame));

			context.drawIndexedIndirect(sc.getDrawCommandBuffer(), 0, sc.getDrawCommandBuffer().getElementCount<DrawIndexedIndirectCommand>(), sizeof(DrawIndexedIndirectCommand));
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

	const Texture2D& ForwardPlus::getLightHeatmap() const {
		return m_debugLightHeatmap;
	}

	void ForwardPlus::setShowHeatmap(bool value) {
		
	}


}