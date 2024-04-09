#include "pch.h"
#include "Graphics/RenderLayers/ShadowRenderLayer.h"

#include "Engine.h"

#include "Graphics/DebugRenderer.h"

namespace sa {

	void ShadowRenderLayer::cleanupRenderData(ShadowRenderData& data) {
		if (data.depthTexture.isValid())
			data.depthTexture.destroy();

		for (uint32_t i = 0; i < data.depthTextureLayers.size(); i++) {
			if(data.depthTextureLayers[i].isValidView())
				data.depthTextureLayers[i].destroy();

			if (data.depthFramebuffers[i] != NULL_RESOURCE) {
				m_renderer.destroyFramebuffer(data.depthFramebuffers[i]);
				data.depthFramebuffers[i] = NULL_RESOURCE;
			}
		}
		data.isInitialized = false;
	}

	void ShadowRenderLayer::initializeRenderData(ShadowRenderData& data, LightType lightType) {
		ShadowPreferences& prefs = getPreferences();
		
		uint32_t res = lightType == LightType::DIRECTIONAL ? prefs.directionalMapResolution : prefs.omniMapResolution;
		Extent extent = { res, res };

		data.depthTexture.create2D(
			TextureUsageFlagBits::DEPTH_ATTACHMENT | TextureUsageFlagBits::SAMPLED,
			extent,
			m_renderer.getDefaultDepthFormat(),
			1,
			ShadowPreferences::MaxCascadeCount,
			1
		);

		uint32_t count = ShadowPreferences::MaxCascadeCount;
		data.depthTexture.createArrayLayerTextures(&count, data.depthTextureLayers.data());

		for (uint32_t i = 0; i < ShadowPreferences::MaxCascadeCount; i++) {
			data.depthFramebuffers[i] = m_renderer.createFramebuffer(m_depthRenderProgram, { data.depthTextureLayers[i] }, data.depthTexture.getExtent());
		}
		data.isInitialized = true;
	}

	void ShadowRenderLayer::renderCascadedShadowMaps(RenderContext& context, const SceneCamera& sceneCamera, ShadowData& data, const ShadowRenderData& renderData, SceneCollection& sceneCollection) {
		const auto& prefs = getPreferences();

		const int cascadeCount = prefs.cascadeCount;
		const float cascadeSplitLambda = prefs.cascadeSplitLambda;

		float clipRange = sceneCamera.getFar() - sceneCamera.getNear();
		float clipRatio = sceneCamera.getFar() / sceneCamera.getNear();

		// Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
		float cascadeSplits[ShadowPreferences::MaxCascadeCount];
		for (uint32_t i = 0; i < cascadeCount; i++) {
			float p = (i + 1) / static_cast<float>(cascadeCount);
			float log = sceneCamera.getNear() * std::pow(clipRatio, p);
			float uniform = sceneCamera.getNear() + clipRange * p;
			float d = cascadeSplitLambda * (log - uniform) + uniform;
			
			cascadeSplits[i] = (d - sceneCamera.getNear()) / clipRange;
			m_cascadeSplits[i] = -(sceneCamera.getNear() + cascadeSplits[i] * clipRange);
		}
		

		static glm::vec3 frustumPoints[8];
		if (m_updateCascades) {
			sceneCamera.calculateFrustumBoundsWorldSpace(frustumPoints);
			//m_updateCascades = false;
		}


		SceneCamera camera;
		camera.setProjectionMode(ProjectionMode::eOrthographic);
		float lastSplitDistance = 0.0f;
		for (uint32_t i = 0; i < cascadeCount; i++) {
			const float splitDistance = cascadeSplits[i];

			glm::vec3 cascadePoints[8];
			memcpy(cascadePoints, frustumPoints, sizeof(glm::vec3) * 8);

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

			const float distance = maxExtents.z * 10.f;
			camera.setPosition(center - glm::vec3(data.lightDirection) * distance);
			camera.lookAt(center);
			camera.setNear(0.0f);
			camera.setFar(distance - minExtents.z);
			camera.setOrthoBounds(sa::Bounds{
				.left = minExtents.x,
				.right = maxExtents.x,
				.top = maxExtents.y,
				.bottom = minExtents.y
			});

			data.lightViewMatrices[i] = camera.getViewMatrix();
			data.lightProjMatrices[i] = camera.getProjectionMatrix();
			data.lightViewMatrices[5][i].x = -(sceneCamera.getNear() + cascadeSplits[i] * clipRange);
			lastSplitDistance = splitDistance;
		}
		data.lightProjMatrices[5] = glm::mat4(1.f);

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
				perFrame.viewMat = data.lightViewMatrices[i];
				perFrame.projMat = data.lightProjMatrices[i];
				perFrame.viewPos = glm::vec4(0);

				if (collection.getDrawCommandBuffer().getElementCount<DrawIndexedIndirectCommand>() > 0) {
					context.pushConstant(ShaderStageFlagBits::VERTEX, perFrame);
					context.drawIndexedIndirect(collection.getDrawCommandBuffer(), 0, collection.getDrawCommandBuffer().getElementCount<DrawIndexedIndirectCommand>(), sizeof(DrawIndexedIndirectCommand));
				}
				context.endRenderProgram(m_depthRenderProgram);
			}
		}
	}

	void ShadowRenderLayer::createSampler() {
		const auto& prefs = getPreferences();
		SamplerInfo info = {};
		info.addressModeU = SamplerAddressMode::CLAMP_TO_BORDER;
		info.addressModeV = SamplerAddressMode::CLAMP_TO_BORDER;
		info.addressModeW = SamplerAddressMode::CLAMP_TO_BORDER;
		info.borderColor = sa::BorderColor::FLOAT_OPAQUE_WHITE;
		info.magFilter =  prefs.smoothShadows ? FilterMode::LINEAR : FilterMode::NEAREST;
		info.minFilter = prefs.smoothShadows ? FilterMode::LINEAR : FilterMode::NEAREST;
		info.compareEnable = true;
		info.compareOp = CompareOp::GREATER;

		m_shadowSampler = m_renderer.createSampler(info);
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
	
		m_shadowShaderDataBuffer.create(BufferType::STORAGE);
		m_shadowTextureCount = 0;

		createSampler();

		const auto& prefs = getPreferences();
		ShadowPreferencesShaderData data = {
			prefs.smoothShadows,
			prefs.cascadeCount
		}; 
		m_preferencesBuffer.create(BufferType::UNIFORM, sizeof(data), &data);


		m_updateCascades = true;
	}

	void ShadowRenderLayer::cleanup() {
		if (m_depthRenderProgram != NULL_RESOURCE) {
			m_renderer.destroyRenderProgram(m_depthRenderProgram);
			m_depthRenderProgram = NULL_RESOURCE;
		}
	}
	
	void ShadowRenderLayer::onRenderTargetResize(UUID renderTargetID, Extent oldExtent, Extent newExtent) {

	}

	void ShadowRenderLayer::onPreferencesUpdated() {
		
		const auto& prefs = getPreferences();
		ShadowPreferencesShaderData data = {
			prefs.smoothShadows,
			prefs.cascadeCount
		};
		std::copy(m_cascadeSplits.begin(), m_cascadeSplits.end(), data.cascadeSplits);
		m_preferencesBuffer.write(data);


		m_renderer.destroySampler(m_shadowSampler);
		createSampler();

		forEachRenderData([](ShadowRenderData& renderData) {
			renderData.isInitialized = false;
		});
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
				cleanupRenderData(renderData);
				initializeRenderData(renderData, data.lightType);
			}

			renderShadowMap(context, *pCamera, data, renderData, sceneCollection);
			
			ShadowShaderData shaderData = {};
			for (uint32_t i = 0; i < data.lightViewMatrices.size(); i++) {
				shaderData.lightMat[i] = data.lightProjMatrices[i] * data.lightViewMatrices[i];
			}
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
	const ResourceID ShadowRenderLayer::getShadowSampler() const {
		return m_shadowSampler;
	}
	const Buffer& ShadowRenderLayer::getPreferencesBuffer() const {
		return m_preferencesBuffer;
	}
}
