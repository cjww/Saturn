#include "pch.h"
#include "ImGuiRenderLayer.h"
namespace sa {

	void ImGuiRenderLayer::init(RenderWindow* pWindow, IRenderTechnique* pRenderTechnique) {
		m_pWindow = pWindow;
		m_pRenderTechnique = pRenderTechnique;

		m_outputTexture = Texture2D(TextureTypeFlagBits::COLOR_ATTACHMENT | TextureTypeFlagBits::SAMPLED, pWindow->getCurrentExtent());

		
		m_imGuiRenderProgram = m_renderer.createRenderProgram()
			.addColorAttachment(true, m_outputTexture)
			.beginSubpass()
			.addAttachmentReference(0, sa::SubpassAttachmentUsage::ColorTarget)
			.endSubpass()
			.end();

		m_imGuiFramebuffer = m_renderer.createFramebuffer(m_imGuiRenderProgram, { m_outputTexture });
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
		m_pRenderTechnique->drawData.finalTexture = m_outputTexture;
		context.beginRenderProgram(m_imGuiRenderProgram, m_imGuiFramebuffer, sa::SubpassContents::DIRECT);
		context.renderImGuiFrame();
		context.endRenderProgram(m_imGuiRenderProgram);
	}

	void ImGuiRenderLayer::onWindowResize(Extent newExtent) {
		Renderer::get().cleanupImGui();
		cleanup();
		init(m_pWindow, m_pRenderTechnique);
	}
	
	const Texture2D& ImGuiRenderLayer::getOutputTexture() const {
		return {};
	}
}
