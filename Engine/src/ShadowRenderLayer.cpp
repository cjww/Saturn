#include "pch.h"
#include "Graphics/RenderLayers/ShadowRenderLayer.h"

#include "Engine.h"

namespace sa {

	void ShadowRenderLayer::cleanupRenderData(ShadowRenderData& data) {
		if (data.depthFramebuffer != NULL_RESOURCE) {
			m_renderer.destroyFramebuffer(data.depthFramebuffer);
		}
		data.isInitialized = false;
	}

	void ShadowRenderLayer::initializeRenderData(const Texture2D& texture, ShadowRenderData& data) {
		data.depthFramebuffer = m_renderer.createFramebuffer(m_depthRenderProgram, { texture });
		data.isInitialized = true;
	}

	void ShadowRenderLayer::renderShadowMap(RenderContext& context, ShadowData data, SceneCollection& sceneCollection) {
		ShadowRenderData& renderData = getRenderTargetData(static_cast<uint32_t>(data.entityID));
		context.beginRenderProgram(m_depthRenderProgram, renderData.depthFramebuffer, SubpassContents::DIRECT);

		for (auto& collection : sceneCollection) {
			if (!collection.readyDescriptorSets(context)) {
				continue;
			}

			// TODO: Can I copy materialshader pipeline, but with custom extent? PS. maybe binding shaders
			context.bindPipeline(m_pipeline);
			context.bindDescriptorSet(collection.getSceneDescriptorSetDepthPass());

			context.bindVertexBuffers(0, { collection.getVertexBuffer() });
			context.bindIndexBuffer(collection.getIndexBuffer());

			Rect viewPort = {};
			viewPort.extent = Renderer::get().getFramebufferExtent(renderData.depthFramebuffer);
			viewPort.offset = { 0, 0 };

			context.setViewport(viewPort);

			glm::mat4 proj = glm::perspective(glm::radians(60.f), 1.f, 0.1f, 100.f);
			glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 30), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

			PerFrameBuffer perFrame = {};
			perFrame.projViewMatrix = proj * view;
			perFrame.viewPos = glm::vec4(0);

			if (collection.getDrawCommandBuffer().getElementCount<DrawIndexedIndirectCommand>() > 0) {
				context.pushConstant(ShaderStageFlagBits::VERTEX, perFrame);
				context.drawIndexedIndirect(collection.getDrawCommandBuffer(), 0, collection.getDrawCommandBuffer().getElementCount<DrawIndexedIndirectCommand>(), sizeof(DrawIndexedIndirectCommand));
			}

		}

		context.endRenderProgram(m_depthRenderProgram);
	}

	void ShadowRenderLayer::init() {
		if (m_depthRenderProgram != NULL_RESOURCE) {
			m_renderer.destroyRenderProgram(m_depthRenderProgram);
			m_depthRenderProgram = NULL_RESOURCE;
		}

		m_depthRenderProgram = m_renderer.createRenderProgram()
			.addDepthAttachment(AttachmentFlagBits::eClear | AttachmentFlagBits::eSampled | AttachmentFlagBits::eStore)
			.beginSubpass()
			.addAttachmentReference(0, SubpassAttachmentUsage::DepthTarget)
			.endSubpass()
			.end();

		m_shaderSet.create({
			sa::ReadSPVFile((Engine::getShaderDirectory() / "ShadowPass.vert.spv").generic_string().c_str())
			});

		PipelineSettings settings = {};
		settings.dynamicStates.push_back(DynamicState::VIEWPORT);
		m_pipeline = m_renderer.createGraphicsPipeline(m_depthRenderProgram, 0, { 256, 256 }, m_shaderSet, settings);
	}

	void ShadowRenderLayer::cleanup() {
		if (m_depthRenderProgram != NULL_RESOURCE) {
			m_renderer.destroyRenderProgram(m_depthRenderProgram);
			m_depthRenderProgram = NULL_RESOURCE;
		}
		if (m_pipeline != NULL_RESOURCE) {
			m_renderer.destroyPipeline(m_pipeline);
			m_pipeline = NULL_RESOURCE;
		}
		if (m_shaderSet.isValid()) {
			m_shaderSet.destroy();
		}
	}
	
	void ShadowRenderLayer::onRenderTargetResize(UUID renderTargetID, Extent oldExtent, Extent newExtent) {

	}

	bool ShadowRenderLayer::preRender(RenderContext& context, SceneCollection& sceneCollection) {
		for (auto it = sceneCollection.iterateShadowsBegin(); it != sceneCollection.iterateShadowsEnd(); it++) {
			sa::ShadowData& data = *it;
			ShadowRenderData& renderData = getRenderTargetData(static_cast<uint32_t>(data.entityID));

			if (renderData.depthFramebuffer == NULL_RESOURCE) {
				renderData.depthFramebuffer = Renderer::get().createFramebuffer(m_depthRenderProgram, { data.shadowmaps[0] });
				SA_DEBUG_LOG_INFO("Created framebuffer ", renderData.depthFramebuffer);
			}
			renderShadowMap(context, data, sceneCollection);
		}
		return true;
	}
	
	bool ShadowRenderLayer::render(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sceneCollection) {

		return true;
	}

	bool ShadowRenderLayer::postRender(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sceneCollection) {

		return true;
	}
}
