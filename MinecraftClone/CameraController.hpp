#pragma once
#include "RenderWindow.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>


class CameraController {
private:
	vr::RenderWindow& window;
	glm::vec3 viewForward;
	glm::vec3 viewPos;
	glm::vec2 lastMousePos;

public:
	float speed = 10.f;
	float sensitivty = 1.f;

	bool mouseLocked;

	CameraController(vr::RenderWindow& window);

	glm::mat4 getView(float dt);
	glm::mat4 getProjection(float fovDegrees);
};