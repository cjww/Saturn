#pragma once

#include "../IRenderLayer.h"

namespace sa {

	struct ShadowRenderData {
		ResourceID depthFramebuffer = NULL_RESOURCE;
	
		bool isInitialized = false;
	};

	struct ShadowPreferences {

	};

	class ShadowRenderLayer : public IRenderLayer<ShadowRenderData, ShadowPreferences> {
	private:

		ResourceID m_depthRenderProgram = NULL_RESOURCE;
		ShaderSet m_shaderSet;
		ResourceID m_pipeline;

		void cleanupRenderData(ShadowRenderData& data);
		void initializeRenderData(const Texture2D& texture, ShadowRenderData& data);

		void renderShadowMap(RenderContext& context, ShadowData data, SceneCollection& sceneCollection);


	public:

		virtual void init() override;
		virtual void cleanup() override;

		virtual void onRenderTargetResize(UUID renderTargetID, Extent oldExtent, Extent newExtent) override;

		virtual bool preRender(RenderContext& context, SceneCollection& sceneCollection) override;
		virtual bool render(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sceneCollection) override;
		virtual bool postRender(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sceneCollection) override;

	};

}