#include "pch.h"
#include "Math.h"

namespace sa::math {

    glm::vec3 worldToScreen(glm::vec3 point, const Camera* camera, glm::vec2 targetScreenPos, glm::vec2 targetScreenSize) {
		glm::mat4 viewMat = camera->getViewMatrix();
		glm::mat4 projMat = camera->getProjectionMatrix();

		glm::vec4 point4 = glm::vec4(point, 1);

		point4 = viewMat * point4;
		point4 = projMat * point4;
		
		point4 /= point4.z;

		point4.x *= targetScreenSize.x / 2;
		point4.y *= targetScreenSize.y / 2;
		
		glm::vec2 center = targetScreenPos + targetScreenSize / 2.0f;
		point4.x += center.x;
		point4.y += center.y;

		
		return point4;
    }
	
}
