#pragma once
#include "Graphics/IRenderLayer.h"

namespace sa {

	class MainRenderLayer : public IRenderLayer {
	private:
		IRenderTechnique* m_pRenderTechnique;

	public:
		MainRenderLayer();

		virtual void init(IRenderTechnique* pRenderTechnique) override;
		virtual void cleanup() override;

		virtual void preRender(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sceneCollection) override;
		virtual void render(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sceneCollection) override;
		virtual void postRender(RenderContext& context) override;

		virtual void onWindowResize(Extent newExtent) override;

		virtual const Texture2D& getOutputTexture() const override;

	};
}

