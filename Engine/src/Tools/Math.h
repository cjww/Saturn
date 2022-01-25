#pragma once
#include "Camera.h"

namespace sa::math {

    glm::vec3 worldToScreen(glm::vec3 point, const Camera* camera, glm::vec2 targetScreenPos, glm::vec2 targetScreenSize);

}