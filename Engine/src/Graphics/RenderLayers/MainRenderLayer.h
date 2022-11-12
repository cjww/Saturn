#pragma once
#include "Graphics/IRenderLayer.h"

namespace sa {

	class MainRenderLayer : public IRenderLayer {
	private:
		RenderWindow* m_pWindow;
		IRenderTechnique* m_pRenderTechnique;


		ResourceID m_sceneFramebuffer;
		Texture2D m_texture;

	public:
		MainRenderLayer();
		MainRenderLayer(Texture2D texture);

		virtual void init(RenderWindow* pWindow, IRenderTechnique* pRenderTechnique) override;
		virtual void cleanup() override;

		virtual void preRender(RenderContext& context, Camera* pCamera) override;
		virtual void render(RenderContext& context, Camera* pCamera) override;
		virtual void postRender(RenderContext& context) override;

		virtual void onWindowResize(Extent newExtent) override;

		virtual const Texture2D& getOutputTexture() const override;

	};
}

