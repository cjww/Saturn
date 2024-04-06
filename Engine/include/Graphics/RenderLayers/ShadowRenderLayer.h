#pragma once

#include "../IRenderLayer.h"

#define MAX_SHADOW_TEXTURE_COUNT 8U

namespace sa {

	struct ShadowRenderData {
		Texture2D depthTexture;
		std::array<Texture2D, 6> depthTextureLayers;

		std::array<ResourceID, 6> depthFramebuffers = { NULL_RESOURCE };
	
		bool isInitialized = false;
	};

	struct ShadowPreferences {
		uint32_t directionalResolution = 1024;
		uint32_t cascadeCount = 3;

		float depthBiasConstant = 0.0f;
		float depthBiasSlope = 0.4f;
		float depthNear = 1.0f;
		float depthFar = 100.f;
	};

	struct alignas(16) ShadowShaderData {
		glm::mat4 lightMat;
		uint32_t mapIndex;
		uint32_t mapCount;
	};

	class ShadowRenderLayer : public IRenderLayer<ShadowRenderData, ShadowPreferences> {
	private:

		ResourceID m_depthRenderProgram = NULL_RESOURCE;
		
		std::array<Texture, MAX_SHADOW_TEXTURE_COUNT> m_shadowTextures;
		uint32_t m_shadowTextureCount;

		DynamicBuffer m_shadowShaderDataBuffer;


		void cleanupRenderData(ShadowRenderData& data);
		void initializeRenderData(ShadowRenderData& data);

		
		void renderShadowMap(RenderContext& context, const glm::vec3& origin, ShadowData& data, const ShadowRenderData& renderData, SceneCollection& sceneCollection);
		
	public:

		virtual void init() override;
		virtual void cleanup() override;

		virtual void onRenderTargetResize(UUID renderTargetID, Extent oldExtent, Extent newExtent) override;

		virtual bool preRender(RenderContext& context, SceneCollection& sceneCollection) override;
		virtual bool render(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sceneCollection) override;
		virtual bool postRender(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sceneCollection) override;

		const Buffer& getShadowDataBuffer() const;
		const std::array<Texture, MAX_SHADOW_TEXTURE_COUNT>& getShadowTextures() const;
		const uint32_t getShadowTextureCount() const;


	};

}