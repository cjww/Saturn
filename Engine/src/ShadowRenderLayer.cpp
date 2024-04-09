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
		data.depthTexture.create2D(
			TextureUsageFlagBits::DEPTH_ATTACHMENT | TextureUsageFlagBits::SAMPLED,
			{ prefs.directionalResolution, prefs.directionalResolution },
			m_renderer.getDefaultDepthFormat(),
			1,
			cascadeCount,
			1
		);

		uint32_t count = cascadeCount;
		data.depthTexture.createArrayLayerTextures(&count, data.depthTextureLayers.data());

		for (uint32_t i = 0; i < cascadeCount; i++) {
			data.depthFramebuffers[i] = m_renderer.createFramebuffer(m_depthRenderProgram, { data.depthTextureLayers[i] }, data.depthTexture.getExtent());
		}
		data.isInitialized = true;
	}

	void ShadowRenderLayer::renderCascadedShadowMaps(RenderContext& context, const SceneCamera& sceneCamera, ShadowData& data, const ShadowRenderData& renderData, SceneCollection& sceneCollection) {
		const auto& prefs = getPreferences();

		SceneCamera camera;
		camera.setAspectRatio(1.f);
		camera.setProjectionMode(ProjectionMode::eOrthographic);

		const int cascadeCount = 4;
		const float cascadeSplitLambda = 0.95f;

		float clipRange = sceneCamera.getFar() - sceneCamera.getNear();
		float clipRatio = sceneCamera.getFar() / sceneCamera.getNear();

		// Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
		float cascadeSplits[cascadeCount];
		for (uint32_t i = 0; i < cascadeCount; i++) {
			float p = (i + 1) / static_cast<float>(cascadeCount);
			float log = sceneCamera.getNear() * std::pow(clipRatio, p);
			float uniform = sceneCamera.getNear() + clipRange * p;
			//float d = cascadeSplitLambda * (log - uniform) + uniform;
			//float d = cascadeSplitLambda * uniform + (1 - cascadeSplitLambda) * log;
			float d = uniform;
			cascadeSplits[i] = (d - sceneCamera.getNear()) / clipRange;
		}

		glm::vec3 frustumPoints[8];
		sceneCamera.calculateFrustumBoundsWorldSpace(frustumPoints);
		float lastSplitDistance = 0.0f;
		for (uint32_t i = 0; i < cascadeCount; i++) {
			glm::vec3 cascadePoints[8];
			memcpy(cascadePoints, frustumPoints, sizeof(glm::vec3) * 8);
			const float splitDistance = cascadeSplits[i];

			// calculate cascade corners
			for (uint32_t j = 0; j < 4; j++) {
				glm::vec3 dist = cascadePoints[j + 4] - cascadePoints[j];
				cascadePoints[j + 4] = cascadePoints[j] + (dist * splitDistance);
				cascadePoints[j] = cascadePoints[j] + (dist * lastSplitDistance);
			}

			// get center
			glm::vec3 center(0.f);
			for (uint32_t j = 0; j < 8; j++) {
				center += cascadePoints[j];
			}
			center /= 8.0f;

			// calculate encapsulation
			float radius = 0.0f;
			for (uint32_t j = 0; j < 8; j++) {
				float distance = glm::length(cascadePoints[j] - center);
				radius = glm::max(radius, distance);
			}
			radius = std::ceil(radius * 16.0f) / 16.0f;

			const glm::vec3 maxExtents(radius);
			const glm::vec3 minExtents(-maxExtents);

			camera.lookAt(center);
			camera.setPosition(center - glm::vec3(data.lightDirection) * -minExtents.z);
			camera.setNear(0.0f);
			camera.setFar(maxExtents.z - minExtents.z);
			camera.setOrthoBounds(sa::Bounds{
				.left = minExtents.x,
				.right = maxExtents.x,
				.top = maxExtents.y,
				.bottom = minExtents.y
			});

			glm::mat4 lightViewMatrix = glm::lookAt(center - glm::vec3(data.lightDirection) * -minExtents.z, center, glm::vec3(0.0f, 1.0f, 0.0f));
			glm::mat4 lightOrthoMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f, maxExtents.z - minExtents.z);
			lightOrthoMatrix[1][1] *= -1.0f;
			//data.lightMatrices[i] = camera.getProjectionMatrix() * camera.getViewMatrix();
			data.lightMatrices[i] = lightOrthoMatrix * lightViewMatrix;
			data.lightMatrices[5][i].x = -(sceneCamera.getNear() + splitDistance * clipRange);

			lastSplitDistance = splitDistance;
		}


		for (auto& collection : sceneCollection) {
			if (!collection.readyDescriptorSets(context)) {
				continue;
			}

			if (!collection.arePipelinesReady()) {
				continue;
			}
			for (uint32_t i = 0; i < cascadeCount; i++) {
				
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

				PerFrameBuffer perFrame = {};
				perFrame.projViewMatrix = data.lightMatrices[i];
				perFrame.viewPos = glm::vec4(0);

				if (collection.getDrawCommandBuffer().getElementCount<DrawIndexedIndirectCommand>() > 0) {
					context.pushConstant(ShaderStageFlagBits::VERTEX, perFrame);
					context.drawIndexedIndirect(collection.getDrawCommandBuffer(), 0, collection.getDrawCommandBuffer().getElementCount<DrawIndexedIndirectCommand>(), sizeof(DrawIndexedIndirectCommand));
				}
				context.endRenderProgram(m_depthRenderProgram);
			}
		}
	}

	void ShadowRenderLayer::renderShadowMap(RenderContext& context, const SceneCamera& sceneCamera, ShadowData& data, const ShadowRenderData& renderData, SceneCollection& sceneCollection) {
		
		switch(data.lightType) {
		case LightType::DIRECTIONAL:
			renderCascadedShadowMaps(context, sceneCamera, data, renderData, sceneCollection);
			break;
		case LightType::POINT:
			break;
		case LightType::SPOT:
			
			break;
		default:
			break;
		}
		

		
	}

	void ShadowRenderLayer::init() {
		if (m_depthRenderProgram != NULL_RESOURCE) {
			m_renderer.destroyRenderProgram(m_depthRenderProgram);
			m_depthRenderProgram = NULL_RESOURCE;
		}

		m_depthRenderProgram = m_renderer.createRenderProgram()
			.addDepthAttachment(AttachmentFlagBits::eClear | AttachmentFlagBits::eSampled | AttachmentFlagBits::eStore, m_renderer.getDefaultDepthFormat())
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
		//TODO point light shadows
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

			renderShadowMap(context, *pCamera, data, renderData, sceneCollection);
			
			ShadowShaderData shaderData = {};
			memcpy(shaderData.lightMat, data.lightMatrices.data(), data.lightMatrices.size() * sizeof(glm::mat4));
			shaderData.mapIndex = m_shadowTextureCount;
			m_shadowShaderDataBuffer << shaderData;

			m_shadowTextures[m_shadowTextureCount++] = renderData.depthTexture;
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
