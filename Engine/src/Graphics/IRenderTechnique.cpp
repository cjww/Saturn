#include "pch.h"
#include "IRenderTechnique.h"

namespace sa {
    
    bool IRenderTechnique::isUsingImGui() const {
        return m_useImGui;
    }

}