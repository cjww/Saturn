#include "pch.h"
#include "Graphics/RenderLayers/ShadowRenderLayer.h"

#include "Engine.h"

namespace sa {

	void ShadowRenderLayer::cleanupRenderData(ShadowRenderData& data) {
		if (!data.isInitialized)
			return;
		data.depthTexture.destroy();
		for (uint32_t i = 0; i < data.depthTextureLayers.size(); i++) {
			data.depthTextureLayers[i].destroy();
			m_renderer.destroyFramebuffer(data.depthFramebuffers[i]);
		}
		data.isInitialized = false;
	}

	void ShadowRenderLayer::initializeRenderData(ShadowRenderData& data) {
		ShadowPreferences& prefs = getPreferences();
		
		const uint32_t cascadeCount = data.depthTextureLayers.size();
		data.depthTexture = Texture2D(
			TextureTypeFlagBits::DEPTH_ATTACHMENT | TextureTypeFlagBits::SAMPLED, 
			{ prefs.directionalResolution, prefs.directionalResolution },
			1,
			1,
			cascadeCount
		);

		uint32_t count = cascadeCount;
		data.depthTexture.createArrayLayerTextures(&count, data.depthTextureLayers.data());

		for (uint32_t i = 0; i < cascadeCount; i++) {
			data.depthFramebuffers[i] = m_renderer.createFramebuffer(m_depthRenderProgram, { data.depthTextureLayers[i] }, data.depthTexture.getExtent());
		}
		data.isInitialized = true;
	}

	void ShadowRenderLayer::renderShadowMap(RenderContext& context, const glm::vec3& origin, ShadowData& data, const ShadowRenderData& renderData, SceneCollection& sceneCollection) {
			
		const auto& prefs = getPreferences();

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
		

		for (auto& collection : sceneCollection) {
			if (!collection.readyDescriptorSets(context)) {
				continue;
			}

			if (!collection.arePipelinesReady()) {
				continue;
			}
			for (uint32_t i = 0; i < renderData.depthTextureLayers.size(); i++) {
				camera.setPosition(origin - glm::vec3(data.lightDirection) * camera.getFar() * 0.5f);


				context.beginRenderProgram(m_depthRenderProgram, renderData.depthFramebuffers[i], SubpassContents::DIRECT);
				collection.bindDepthPipeline(context);
				context.bindDescriptorSet(collection.getSceneDescriptorSetDepthPass());

				context.bindVertexBuffers(0, { collection.getVertexBuffer() });
				context.bindIndexBuffer(collection.getIndexBuffer());

				Rect viewport = {};
				viewport.extent = Renderer::get().getFramebufferExtent(renderData.depthFramebuffers[i]);
				viewport.offset = { 0, 0 };
			
				context.setViewport(viewport);
				context.setScissor(viewport);

				context.setDepthBiasEnable(true);
				context.setDepthBias(prefs.depthBiasConstant, 0.0f, prefs.depthBiasSlope);

				context.setCullMode(sa::CullModeFlagBits::FRONT);

				data.lightMatrix = camera.getProjectionMatrix() * camera.getViewMatrix();

				PerFrameBuffer perFrame = {};
				perFrame.projViewMatrix = data.lightMatrix;
				perFrame.viewPos = glm::vec4(0);

				if (collection.getDrawCommandBuffer().getElementCount<DrawIndexedIndirectCommand>() > 0) {
					context.pushConstant(ShaderStageFlagBits::VERTEX, perFrame);
					context.drawIndexedIndirect(collection.getDrawCommandBuffer(), 0, collection.getDrawCommandBuffer().getElementCount<DrawIndexedIndirectCommand>(), sizeof(DrawIndexedIndirectCommand));
				}
				context.endRenderProgram(m_depthRenderProgram);
			}
		}
		
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
	
		m_shadowShaderDataBuffer = m_renderer.createDynamicBuffer(BufferType::STORAGE);
		m_shadowTextureCount = 0;
	}

	void ShadowRenderLayer::cleanup() {
		if (m_depthRenderProgram != NULL_RESOURCE) {
			m_renderer.destroyRenderProgram(m_depthRenderProgram);
			m_depthRenderProgram = NULL_RESOURCE;
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
				initializeRenderData(renderData);
			}

			renderShadowMap(context, data.lightPosition, data, renderData, sceneCollection);

			ShadowShaderData shaderData = {};
			shaderData.lightMat = data.lightMatrix;
			shaderData.mapIndex = m_shadowTextureCount;
			shaderData.mapCount = 1;
			m_shadowShaderDataBuffer << shaderData;

			m_shadowTextures[m_shadowTextureCount++] = renderData.depthTextureLayers[0];
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
				initializeRenderData(renderData);
			}

			renderShadowMap(context, pCamera->getPosition(), data, renderData, sceneCollection);
			
			ShadowShaderData shaderData = {};
			shaderData.lightMat = data.lightMatrix;
			shaderData.mapIndex = m_shadowTextureCount;
			shaderData.mapCount = 1;
			m_shadowShaderDataBuffer << shaderData;

			m_shadowTextures[m_shadowTextureCount++] = renderData.depthTextureLayers[0];
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
