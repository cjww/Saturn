#include "ImGuiRenderLayer.h"

ImGuiRenderLayer::ImGuiRenderLayer(sa::RenderWindow* pWindow)
	: m_renderer(sa::Renderer::get())
	, m_pWindow(pWindow)
{
	init();
}

void ImGuiRenderLayer::init() {
		
	m_imGuiRenderProgram = m_renderer.createRenderProgram()
		.addSwapchainAttachment(m_pWindow->getSwapchainID())
		.beginSubpass()
		.addAttachmentReference(0, sa::SubpassAttachmentUsage::ColorTarget)
		.endSubpass()
		.end();

	std::vector<sa::Texture> textures;
	m_imGuiFramebuffer = m_renderer.createSwapchainFramebuffer(m_imGuiRenderProgram, m_pWindow->getSwapchainID(), textures);
	m_renderer.initImGui(*m_pWindow, m_imGuiRenderProgram, 0);
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

void ImGuiRenderLayer::render(sa::RenderContext& context, const sa::Texture& texture) {
	context.beginRenderProgram(m_imGuiRenderProgram, m_imGuiFramebuffer, sa::SubpassContents::DIRECT);
	context.renderImGuiFrame();
	context.endRenderProgram(m_imGuiRenderProgram);
}

void ImGuiRenderLayer::onWindowResize(sa::Extent newExtent) {
	sa::Renderer::get().cleanupImGui();
	cleanup();
	init();
}