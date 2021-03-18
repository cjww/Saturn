#include "CameraController.hpp"

CameraController::CameraController(vr::RenderWindow& window) : window(window) {
	viewForward = glm::vec3(0, 0, 1);
	viewPos = glm::vec3(0.f);

	mouseLocked = true;

	glm::vec2 center = glm::vec2(window.getCurrentExtent().width / 2, window.getCurrentExtent().height / 2);
	window.setCursorPosition(center);

	window.setHideCursor(true);

	sensitivty = 0.001f;
	speed = 0.01f;
}

glm::mat4 CameraController::getView(float dt) {
	int hori = glfwGetKey(window.getWindowHandle(), GLFW_KEY_D) - glfwGetKey(window.getWindowHandle(), GLFW_KEY_A);
	int vert = glfwGetKey(window.getWindowHandle(), GLFW_KEY_W) - glfwGetKey(window.getWindowHandle(), GLFW_KEY_S);

	glm::vec2 mPos = window.getCursorPosition();
	glm::vec2 center = glm::vec2(window.getCurrentExtent().width / 2, window.getCurrentExtent().height / 2);
	glm::vec2 diff = mPos - center;
	if (mouseLocked) {
		window.setCursorPosition(center);
		window.setHideCursor(true);
	}
	else {
		diff = glm::vec2(0, 0);
	}

	if (glfwGetKey(window.getWindowHandle(), GLFW_KEY_ESCAPE)) {
		mouseLocked = false;
		window.setHideCursor(false);
	}

	int up = glfwGetKey(window.getWindowHandle(), GLFW_KEY_SPACE) - glfwGetKey(window.getWindowHandle(), GLFW_KEY_LEFT_CONTROL);
	bool sprint = glfwGetKey(window.getWindowHandle(), GLFW_KEY_LEFT_SHIFT);

	viewForward = glm::vec3(glm::vec4(viewForward, 0.0f) * glm::rotate(diff.x * dt * sensitivty, glm::vec3(0, 1, 0)));

	glm::vec3 right = glm::cross(viewForward, glm::vec3(0, 1, 0));
	viewForward = glm::vec3(glm::vec4(viewForward, 0.0f) * glm::rotate(diff.y * dt * sensitivty, right));
	viewForward = normalize(viewForward);

	//viewForward = glm::vec3(0, 0, 1);

	float finalSpeed = speed;
	if (sprint) {
		finalSpeed *= 2;
	}

	viewPos += right * (float)hori * dt * finalSpeed;
	viewPos += viewForward * (float)vert * dt * finalSpeed;
	viewPos += glm::vec3(0, 1, 0) * (float)up * dt * finalSpeed;


	return glm::lookAt(viewPos, viewPos + viewForward, glm::vec3(0, 1, 0));
}

glm::mat4 CameraController::getProjection(float fovDegrees) {
	auto projection = glm::perspective(glm::radians(fovDegrees), (float)window.getCurrentExtent().width / window.getCurrentExtent().height, 0.01f, 1000.0f);
	projection[1][1] *= -1;
	return projection;
}