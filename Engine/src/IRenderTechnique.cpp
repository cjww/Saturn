#include "IRenderTechnique.h"

namespace sa {

    bool IRenderTechnique::isUsingImGui() const {
        return m_useImGui;
    }

    void IRenderTechnique::addCamera(Camera* camera) {
        m_activeCameras.insert(camera);
    }

    void IRenderTechnique::removeCamera(Camera* camera) {
        m_activeCameras.erase(camera);
    }
}