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

		Texture2D m_outputTexture;

	public:
		virtual void init(RenderWindow* pWindow, IRenderTechnique* pRenderTechnique) override;
		virtual void cleanup();

		virtual void postRender(RenderContext& context) override;

		virtual void onWindowResize(Extent newExtent) override;

		virtual const Texture2D& getOutputTexture() const override;

	};
}

