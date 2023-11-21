#include "pch.h"
#include "Graphics/RenderTechniques/ForwardPlus.h"

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
		m_lightCullingShader.create({ code });
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
		m_debugHeatmapShaderSet.create({
			sa::ReadSPVFile((Engine::getShaderDirectory() / "DebugHeatmap.vert.spv").generic_string().c_str()),
			sa::ReadSPVFile((Engine::getShaderDirectory() / "DebugHeatmap.frag.spv").generic_string().c_str())
		});

	}

	void ForwardPlus::cleanup() {
		m_lightCullingShader.destroy();
	}

	bool ForwardPlus::preRender(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sc) {
		SA_PROFILE_FUNCTION();
		if (!pCamera)
			return false;

		const RenderTarget::MainRenderData& data = pRenderTarget->getMainRenderData();

		if (!data.isInitialized) {
			pRenderTarget->cleanupMainRenderData();
			pRenderTarget->initializeMainRenderData(m_colorRenderProgram, m_depthPreRenderProgram, m_lightCullingShader, m_debugHeatmapShaderSet, m_linearSampler, pRenderTarget->getExtent());
		}
		
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


		context.beginRenderProgram(m_depthPreRenderProgram, data.depthFramebuffer, SubpassContents::DIRECT);
		for (auto& collection : sc) {

			if (!collection.getMaterialShader()->isLoaded()) {
				continue;
			}
			collection.readyDescriptorSets();
			collection.recreatePipelines(m_colorRenderProgram, m_depthPreRenderProgram, pRenderTarget->getExtent());


			context.updateDescriptorSet(collection.getSceneDescriptorSetDepthPass(), 0, collection.getObjectBuffer());

			context.updateDescriptorSet(collection.getSceneDescriptorSetColorPass(), 0, collection.getObjectBuffer());
			context.updateDescriptorSet(collection.getSceneDescriptorSetColorPass(), 1, sc.getLightBuffer());
			context.updateDescriptorSet(collection.getSceneDescriptorSetColorPass(), 2, collection.getMaterialBuffer());
			context.updateDescriptorSet(collection.getSceneDescriptorSetColorPass(), 3, collection.getMaterialIndicesBuffer());
			context.updateDescriptorSet(collection.getSceneDescriptorSetColorPass(), 4, data.lightIndexBuffer.getBuffer());
			context.updateDescriptorSet(collection.getSceneDescriptorSetColorPass(), 5, m_linearSampler);
			context.updateDescriptorSet(collection.getSceneDescriptorSetColorPass(), 6, collection.getTextures(), 0);

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
		context.bindPipeline(m_lightCullingPipeline);
		context.bindDescriptorSet(data.lightCullingDescriptorSet);

		context.pushConstant(ShaderStageFlagBits::COMPUTE, pCamera->getProjectionMatrix());
		context.pushConstant(ShaderStageFlagBits::COMPUTE, pCamera->getViewMatrix(), sizeof(Matrix4x4));

		context.dispatch(data.tileCount.x, data.tileCount.y, 1);

		//context.barrierColorCompute(m_lightBuffer);
		// TODO generate shadowMaps
		return true;
	}


	const Texture& ForwardPlus::render(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sc) {
		SA_PROFILE_FUNCTION();
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
		for (auto& collection : sc) {
			if (!collection.getMaterialShader()->isLoaded()) {
				continue;
			}
			collection.bindColorPipeline(context);

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
		return data.colorTexture.getTexture();
	}

}