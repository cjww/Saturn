#include "pch.h"
#include "IRenderTechnique.h"

namespace sa {
    void IRenderTechnique::drawImGui(RenderContext& context) {
        context.beginRenderProgram(m_imGuiRenderProgram, m_imGuiFramebuffer, sa::SubpassContents::DIRECT);
        context.renderImGuiFrame();
        context.endRenderProgram(m_imGuiRenderProgram);
    }
    bool IRenderTechnique::isUsingImGui() const {
        return m_useImGui;
    }

    sa::Extent IRenderTechnique::getCurrentExtent() const {
        return m_pWindow->getCurrentExtent();
    }

    void IRenderTechnique::setupImGuiPass() {
        m_imGuiRenderProgram = m_renderer.createRenderProgram()
            .addSwapchainAttachment(m_pWindow->getSwapchainID())
            .beginSubpass()
            .addAttachmentReference(0, sa::SubpassAttachmentUsage::ColorTarget)
            .endSubpass()
            .end();

        m_imGuiFramebuffer = m_renderer.createSwapchainFramebuffer(m_imGuiRenderProgram, m_pWindow->getSwapchainID(), {});
        m_renderer.initImGui(*m_pWindow, m_imGuiRenderProgram, 0);

    }

    IRenderTechnique::IRenderTechnique()
        : m_renderer(Renderer::get())
    {

    }

}