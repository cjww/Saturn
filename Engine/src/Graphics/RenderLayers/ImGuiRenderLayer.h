#pragma once
#include "Graphics/IRenderLayer.h"
namespace sa {

	class ImGuiRenderLayer : public IRenderLayer {
	private:
		ResourceID m_imGuiRenderProgram;
		ResourceID m_imGuiFramebuffer;
		RenderWindow* m_pWindow;
	public:
		virtual void init(RenderWindow* pWindow, IRenderLayer*) override;
		virtual void cleanup();

		virtual void postRender(RenderContext& context) override;

		virtual void onWindowResize(Extent newExtent) override;
	};
}

