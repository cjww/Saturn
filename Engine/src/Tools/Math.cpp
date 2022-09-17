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

	glm::vec3 screenToWorld(glm::vec2 point, const Camera* camera, glm::vec2 screenPos, glm::vec2 screenSize) {

		point = screenPos - point; // relative to screen
		point += screenSize * 0.5f; // translate
		point /= screenSize * 0.5f;
		point *= -1.f;

		glm::vec4 point3D = glm::vec4(point, -1.f, 1.0f);
		glm::mat4 viewMat = camera->getViewMatrix();
		glm::mat4 projMat = camera->getProjectionMatrix();

		glm::mat4 screenToWorldMat = projMat * viewMat;
		screenToWorldMat = glm::inverse(screenToWorldMat);

		point3D = screenToWorldMat * point3D;
		
		point3D /= point3D.w;
		
		return glm::vec3(point3D);
	}
	
}
