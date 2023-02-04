#include "pch.h"
#include "IRenderTechnique.h"

namespace sa {
    IRenderTechnique::IRenderTechnique()
        : m_renderer(Renderer::get())
    {

    }
}