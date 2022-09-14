#include "pch.h"
#include "ImGuiRenderLayer.h"
namespace sa {

	void ImGuiRenderLayer::init(RenderWindow* pWindow, IRenderLayer*) {
		m_pWindow = pWindow;
		Renderer& renderer = Renderer::get();
		m_imGuiRenderProgram = renderer.createRenderProgram()
			.addSwapchainAttachment(pWindow->getSwapchainID())
			.beginSubpass()
			.addAttachmentReference(0, sa::SubpassAttachmentUsage::ColorTarget)
			.endSubpass()
			.end();

		m_imGuiFramebuffer = renderer.createSwapchainFramebuffer(m_imGuiRenderProgram, pWindow->getSwapchainID(), {});
		renderer.initImGui(*pWindow, m_imGuiRenderProgram, 0);
	}

	void ImGuiRenderLayer::cleanup() {

	}

	void ImGuiRenderLayer::postRender(RenderContext& context) {
		context.beginRenderProgram(m_imGuiRenderProgram, m_imGuiFramebuffer, sa::SubpassContents::DIRECT);
		context.renderImGuiFrame();
		context.endRenderProgram(m_imGuiRenderProgram);
	}

	void ImGuiRenderLayer::onWindowResize(Extent newExtent) {
		Renderer::get().cleanupImGui();
		init(m_pWindow, nullptr);
	}
}
