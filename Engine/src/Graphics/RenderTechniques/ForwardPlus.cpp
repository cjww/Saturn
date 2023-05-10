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

		auto code = ReadSPVFile((Engine::getShaderDirectory() / "LightCulling.comp.spv").generic_string().c_str());
		m_lightCullingShader = m_renderer.createShaderSet({ code });
		m_lightCullingPipeline = m_renderer.createComputePipeline(m_lightCullingShader);
		
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

	ForwardPlus::ForwardPlus() 
		: m_renderer(sa::Renderer::get())
	{

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

	bool ForwardPlus::preRender(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sc) {
		if (!pCamera)
			return false;

		const RenderTarget::MainRenderData& data = pRenderTarget->getMainRenderData();

		if (!data.isInitialized) {
			pRenderTarget->cleanupMainRenderData();
			pRenderTarget->initializeMainRenderData(m_colorRenderProgram, m_depthPreRenderProgram, m_lightCullingShader, m_linearSampler, pRenderTarget->getExtent());
		}

		context.updateDescriptorSet(data.sceneDepthDescriptorSet, 0, sc.getObjectBuffer());

		context.updateDescriptorSet(data.sceneDescriptorSet, 0, sc.getObjectBuffer());
		context.updateDescriptorSet(data.sceneDescriptorSet, 1, sc.getLightBuffer());
		context.updateDescriptorSet(data.sceneDescriptorSet, 2, sc.getMaterialBuffer());
		context.updateDescriptorSet(data.sceneDescriptorSet, 3, sc.getMaterialIndicesBuffer());
		context.updateDescriptorSet(data.sceneDescriptorSet, 4, data.lightIndexBuffer.getBuffer());
		context.updateDescriptorSet(data.sceneDescriptorSet, 6, sc.getTextures(), 0);

		context.updateDescriptorSet(data.lightCullingDescriptorSet, 1, data.lightIndexBuffer.getBuffer());
		context.updateDescriptorSet(data.lightCullingDescriptorSet, 2, sc.getLightBuffer());

		context.bindVertexBuffers(0, { sc.getVertexBuffer() });
		context.bindIndexBuffer(sc.getIndexBuffer());

		Rectf cameraViewport = pCamera->getViewport();
		Rect viewport = {
			{ cameraViewport.offset.x * pRenderTarget->getExtent().width, cameraViewport.offset.y * pRenderTarget->getExtent().height },
			{ cameraViewport.extent.x * pRenderTarget->getExtent().width, cameraViewport.extent.y * pRenderTarget->getExtent().height }
		};
		if ((viewport.extent.height & viewport.extent.width) == 0) {
			return {};
		}
		pCamera->setAspectRatio((float)viewport.extent.width / viewport.extent.height);
		Matrix4x4 projViewMat = pCamera->getProjectionMatrix() * pCamera->getViewMatrix();

		PerFrameBuffer perFrame;
		perFrame.projViewMatrix = projViewMat;
		perFrame.viewPos = pCamera->getPosition();


		// Depth prepass
		context.beginRenderProgram(m_depthPreRenderProgram, data.depthFramebuffer, SubpassContents::DIRECT);
		context.bindPipeline(data.depthPipeline);

		context.setViewport(viewport);
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


	const Texture& ForwardPlus::render(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sc) {
		if (!pCamera)
			return {};
		const RenderTarget::MainRenderData& data = pRenderTarget->getMainRenderData();

		Rectf cameraViewport = pCamera->getViewport();
		Rect viewport = {
			{ cameraViewport.offset.x * pRenderTarget->getExtent().width, cameraViewport.offset.y * pRenderTarget->getExtent().height },
			{ cameraViewport.extent.x * pRenderTarget->getExtent().width, cameraViewport.extent.y * pRenderTarget->getExtent().height }
		};
		if ((viewport.extent.height & viewport.extent.width) == 0) {
			return {};
		}
		pCamera->setAspectRatio((float)viewport.extent.width / viewport.extent.height);
		Matrix4x4 projViewMat = pCamera->getProjectionMatrix() * pCamera->getViewMatrix();

		PerFrameBuffer perFrame;
		perFrame.projViewMatrix = projViewMat;
		perFrame.viewPos = pCamera->getPosition();


		// Main color pass
		context.beginRenderProgram(m_colorRenderProgram, data.colorFramebuffer, SubpassContents::DIRECT);
		context.bindPipeline(data.colorPipeline);
		context.bindDescriptorSet(data.sceneDescriptorSet, data.colorPipeline);

		context.setViewport(viewport);

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

		pRenderTarget->setOutputTexture(data.colorTexture);
		return data.colorTexture.getTexture();
	}

	const Texture2D& ForwardPlus::getLightHeatmap() const {
		return m_debugLightHeatmap;
	}

	void ForwardPlus::setShowHeatmap(bool value) {
		
	}


}