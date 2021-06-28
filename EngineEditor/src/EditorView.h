#pragma once
#include <Camera.h>

class EditorView {
private:
	Camera m_camera;
	RenderWindow* m_pWindow;

	glm::vec2 m_lastMousePos;
	float m_mouseSensitivity;
	float m_moveSpeed;

public:
	EditorView(RenderWindow* pWindow);
	~EditorView();

	// moves camera around scene according to input
	void update(float dt);

	Camera* getCamera();

};