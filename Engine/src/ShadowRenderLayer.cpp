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

	void ShadowRenderLayer::renderShadowMap(RenderContext& context, const glm::vec3& origin, ShadowData& data, ResourceID framebuffer, SceneCollection& sceneCollection) {
		
		context.beginRenderProgram(m_depthRenderProgram, framebuffer, SubpassContents::DIRECT);

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
			viewPort.extent = Renderer::get().getFramebufferExtent(framebuffer);
			viewPort.offset = { 0, 0 };
			
			context.setViewport(viewPort);
			context.setDepthBiasEnable(true);
			const auto& prefs = getPreferences();
			context.setDepthBias(prefs.depthBiasConstant, 0.0f, prefs.depthBiasSlope);

			SceneCamera camera;
			camera.setAspectRatio(1.f);
			camera.setNear(prefs.depthNear);
			camera.setFar(prefs.depthFar);
			switch (data.lightType) {
			case LightType::DIRECTIONAL:
				camera.setProjectionMode(ProjectionMode::eOrthographic);
				camera.setOrthoWidth(64);
				camera.setPosition(origin - glm::vec3(data.lightDirection) * camera.getFar() * 0.5f);
				camera.setForward(data.lightDirection);
				break;
			case LightType::POINT:
				break;
			case LightType::SPOT:
				camera.setProjectionMode(ProjectionMode::ePerspective);
				camera.setFOVDegrees(45.f);
				camera.setPosition(data.lightPosition);
				camera.setForward(data.lightDirection);

				break;
			default:
				break;
			}

			data.lightMatrix = camera.getProjectionMatrix() * camera.getViewMatrix();

			PerFrameBuffer perFrame = {};
			perFrame.projViewMatrix = data.lightMatrix;
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
		settings.dynamicStates.push_back(DynamicState::DEPTH_BIAS);
		settings.dynamicStates.push_back(DynamicState::DEPTH_BIAS_ENABLE);
		settings.cullMode = CullModeFlagBits::FRONT;
		
		ShadowPreferences& prefs = getPreferences();
		prefs.directionalResolution = 526;

		
		m_pipeline = m_renderer.createGraphicsPipeline(m_depthRenderProgram, 0, { prefs.directionalResolution, prefs.directionalResolution }, m_shaderSet, settings);
	
		m_shadowShaderDataBuffer = m_renderer.createDynamicBuffer(BufferType::STORAGE);
		m_shadowTextureCount = 0;
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
		SA_PROFILE_FUNCTION();
		for (auto it = sceneCollection.iterateShadowsBegin(); it != sceneCollection.iterateShadowsEnd(); it++) {
			if (m_shadowTextureCount >= MAX_SHADOW_TEXTURE_COUNT)
				break; 

			sa::ShadowData& data = *it;
			if (data.lightType == LightType::DIRECTIONAL)
				continue;

			ShadowRenderData& renderData = getRenderTargetData(static_cast<uint32_t>(data.entityID));

			if (!renderData.isInitialized) {
				ShadowPreferences& prefs = getPreferences();
				renderData.depthTextures[0] = Texture2D(TextureTypeFlagBits::DEPTH_ATTACHMENT | TextureTypeFlagBits::SAMPLED, { prefs.directionalResolution, prefs.directionalResolution });
				renderData.depthTextureCount = 1;
				renderData.depthFramebuffer = Renderer::get().createFramebuffer(m_depthRenderProgram, { renderData.depthTextures[0] });
				renderData.isInitialized = true;
			}

			renderShadowMap(context, data.lightPosition, data, renderData.depthFramebuffer, sceneCollection);

			ShadowShaderData shaderData = {};
			shaderData.lightMat = data.lightMatrix;
			shaderData.mapIndex = m_shadowTextureCount;
			shaderData.mapCount = 1;
			m_shadowShaderDataBuffer << shaderData;

			m_shadowTextures[m_shadowTextureCount++] = renderData.depthTextures[0];
		}
		return true;
	}
	
	bool ShadowRenderLayer::render(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sceneCollection) {
		SA_PROFILE_FUNCTION();
		for (auto it = sceneCollection.iterateShadowsBegin(); it != sceneCollection.iterateShadowsEnd(); it++) {
			if (m_shadowTextureCount >= MAX_SHADOW_TEXTURE_COUNT)
				break;

			sa::ShadowData& data = *it;
			if (data.lightType != LightType::DIRECTIONAL)
				continue;

			ShadowRenderData& renderData = getRenderTargetData(pRenderTarget->getID() ^ static_cast<uint32_t>(data.entityID));

			if (!renderData.isInitialized) {
				ShadowPreferences& prefs = getPreferences();

				renderData.depthTextures[0] = Texture2D(TextureTypeFlagBits::DEPTH_ATTACHMENT | TextureTypeFlagBits::SAMPLED, { prefs.directionalResolution, prefs.directionalResolution });
				renderData.depthTextureCount = 1;
				renderData.depthFramebuffer = Renderer::get().createFramebuffer(m_depthRenderProgram, { renderData.depthTextures[0] });
				renderData.isInitialized = true;
			}

			renderShadowMap(context, pCamera->getPosition(), data, renderData.depthFramebuffer, sceneCollection);
			
			ShadowShaderData shaderData = {};
			shaderData.lightMat = data.lightMatrix;
			shaderData.mapIndex = m_shadowTextureCount;
			shaderData.mapCount = 1;
			m_shadowShaderDataBuffer << shaderData;

			m_shadowTextures[m_shadowTextureCount++] = renderData.depthTextures[0];
		}
		return true;
	}

	bool ShadowRenderLayer::postRender(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sceneCollection) {
		m_shadowShaderDataBuffer.swap();
		m_shadowShaderDataBuffer.clear();
		m_shadowTextureCount = 0;
		return true;
	}
	
	const Buffer& ShadowRenderLayer::getShadowDataBuffer() const {
		return m_shadowShaderDataBuffer.getBuffer();
	}

	const std::array<Texture, MAX_SHADOW_TEXTURE_COUNT>& ShadowRenderLayer::getShadowTextures() const {
		return m_shadowTextures;
	}

	const uint32_t ShadowRenderLayer::getShadowTextureCount() const {
		return m_shadowTextureCount;
	}
}
