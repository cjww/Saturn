#pragma once
#include "Graphics/IRenderLayer.h"
#include "Resources/DynamicTexture.hpp"
namespace sa {

	class ImGuiRenderLayer : public IRenderLayer {
	private:
		ResourceID m_imGuiRenderProgram = NULL_RESOURCE;
		ResourceID m_imGuiFramebuffer = NULL_RESOURCE;
		
		RenderWindow* m_pWindow;

		IRenderTechnique* m_pRenderTechnique;

		DynamicTexture2D m_outputTexture;

		RenderTarget m_renderTarget;

	public:
		ImGuiRenderLayer(RenderWindow* pWindow);

		virtual void init(IRenderTechnique* pRenderTechnique) override;
		virtual void cleanup();

		virtual void postRender(RenderContext& context) override;

		virtual void onWindowResize(Extent newExtent) override;

		virtual const Texture2D& getOutputTexture() const override;

	};
}

