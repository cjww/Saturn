#include "EditorView.h"

EditorView::EditorView(RenderWindow* pWindow)
	: m_pWindow(pWindow) 
{
	Rect viewport;
	viewport.setSize(pWindow->getCurrentExtent());
	viewport.setPosition({ 0, 0 });
	m_camera.setViewport(viewport);

	m_camera.setPosition(glm::vec3(0, 0, 1));
	m_camera.lookAt(glm::vec3(0, 0, 0));

	m_mouseSensitivity = 4.0f;
	m_moveSpeed = 4.0f;
}

EditorView::~EditorView() {

}

#include "imgui.h"
void EditorView::update(float dt) {
	
	if (m_pWindow->getMouseButton(GLFW_MOUSE_BUTTON_1)) {
		glm::vec mousePos = m_pWindow->getCursorPosition();
		glm::vec2 delta = m_lastMousePos - mousePos;
		m_camera.rotate(glm::radians(delta.x) * dt * m_mouseSensitivity, glm::vec3(0, 1, 0));
		m_camera.rotate(glm::radians(delta.y) * dt * m_mouseSensitivity, -m_camera.getRight());
		m_lastMousePos = mousePos;
	}
	else {
		m_lastMousePos = m_pWindow->getCursorPosition();
	}

	int forward = m_pWindow->getKey(GLFW_KEY_W) - m_pWindow->getKey(GLFW_KEY_S);
	int right = m_pWindow->getKey(GLFW_KEY_D) - m_pWindow->getKey(GLFW_KEY_A);
	int up = m_pWindow->getKey(GLFW_KEY_E) - m_pWindow->getKey(GLFW_KEY_Q);

	glm::vec3 camPos = m_camera.getPosition();
	camPos += (float)forward * dt * m_moveSpeed * m_camera.getForward();
	camPos += (float)right * dt * m_moveSpeed * m_camera.getRight();
	camPos += (float)up * dt * m_moveSpeed * m_camera.getUp();

	m_camera.setPosition(camPos);

}

Camera* EditorView::getCamera() {
	return &m_camera;
}
