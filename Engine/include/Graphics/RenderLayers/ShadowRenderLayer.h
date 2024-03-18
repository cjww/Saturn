#pragma once

#include "../IRenderLayer.h"

#define MAX_SHADOW_TEXTURE_COUNT 8U

namespace sa {

	struct ShadowRenderData {
		std::array<Texture2D, 4> depthTextures;
		uint8_t depthTextureCount;
		ResourceID depthFramebuffer = NULL_RESOURCE;
	
		bool isInitialized = false;
	};

	struct ShadowPreferences {

	};

	struct alignas(16) ShadowShaderData {
		glm::mat4 lightMat;
		uint32_t mapIndex;
		uint32_t mapCount;
	};

	class ShadowRenderLayer : public IRenderLayer<ShadowRenderData, ShadowPreferences> {
	private:

		ResourceID m_depthRenderProgram = NULL_RESOURCE;
		ShaderSet m_shaderSet;
		ResourceID m_pipeline;


		std::array<Texture, MAX_SHADOW_TEXTURE_COUNT> m_shadowTextures;
		uint32_t m_shadowTextureCount;

		DynamicBuffer m_shadowShaderDataBuffer;


		void cleanupRenderData(ShadowRenderData& data);
		void initializeRenderData(const Texture2D& texture, ShadowRenderData& data);

		
		void renderShadowMap(RenderContext& context, const glm::vec3& origin, ShadowData& data, ResourceID framebuffer, SceneCollection& sceneCollection);


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