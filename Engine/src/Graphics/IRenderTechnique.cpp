#include "pch.h"
#include "IRenderTechnique.h"

namespace sa {
    IRenderTechnique::IRenderTechnique(bool renderToSwapchain)
        : m_renderer(Renderer::get())
        , m_isRenderingToSwapchain(renderToSwapchain)
    {

    }

    const Texture& IRenderTechnique::getOutputTexture() const {
        return m_outputTexture;
    }

    sa::Extent IRenderTechnique::getCurrentExtent() const {
        return m_pWindow->getCurrentExtent();
    }

}