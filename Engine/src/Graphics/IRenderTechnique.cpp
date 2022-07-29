#include "pch.h"
#include "IRenderTechnique.h"

namespace sa {
    
    bool IRenderTechnique::isUsingImGui() const {
        return m_useImGui;
    }

    sa::Extent IRenderTechnique::getCurrentExtent() const {
        return m_pWindow->getCurrentExtent();
    }

}