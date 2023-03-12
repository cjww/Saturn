#pragma once
#include "SceneCamera.h"

namespace sa::math {

    glm::vec3 worldToScreen(const glm::vec3& point, const SceneCamera* camera, glm::vec2 targetScreenPos, glm::vec2 targetScreenSize);
    glm::vec3 screenToWorld(glm::vec2 point, const SceneCamera* camera, glm::vec2 screenPos, glm::vec2 screenSize);
    
}