#include "pch.h"
#include "IRenderTechnique.h"

namespace sa {
    IRenderTechnique::IRenderTechnique(bool renderToSwapchain)
        : m_isRenderingToSwapchain(renderToSwapchain)
    {

    }

    sa::Extent IRenderTechnique::getCurrentExtent() const {
        return m_pWindow->getCurrentExtent();
    }

}