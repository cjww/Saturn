#include "pch.h"
#include "ImGuiRenderLayer.h"
namespace sa {

	void ImGuiRenderLayer::init(RenderWindow* pWindow, IRenderLayer*) {
		m_pWindow = pWindow;
		
		m_imGuiRenderProgram = m_renderer.createRenderProgram()
			.addSwapchainAttachment(pWindow->getSwapchainID())
			.beginSubpass()
			.addAttachmentReference(0, sa::SubpassAttachmentUsage::ColorTarget)
			.endSubpass()
			.end();

		m_imGuiFramebuffer = m_renderer.createSwapchainFramebuffer(m_imGuiRenderProgram, pWindow->getSwapchainID(), {});
		m_renderer.initImGui(*pWindow, m_imGuiRenderProgram, 0);
	}

	void ImGuiRenderLayer::cleanup() {
		if (m_imGuiRenderProgram != NULL_RESOURCE) {
			m_renderer.destroyRenderProgram(m_imGuiRenderProgram);
			m_imGuiRenderProgram = NULL_RESOURCE;
		}
		if (m_imGuiFramebuffer != NULL_RESOURCE) {
			m_renderer.destroyFramebuffer(m_imGuiFramebuffer);
			m_imGuiFramebuffer = NULL_RESOURCE;
		}
	}

	void ImGuiRenderLayer::postRender(RenderContext& context) {
		context.beginRenderProgram(m_imGuiRenderProgram, m_imGuiFramebuffer, sa::SubpassContents::DIRECT);
		context.renderImGuiFrame();
		context.endRenderProgram(m_imGuiRenderProgram);
	}

	void ImGuiRenderLayer::onWindowResize(Extent newExtent) {
		Renderer::get().cleanupImGui();
		cleanup();
		init(m_pWindow, nullptr);
	}
}
