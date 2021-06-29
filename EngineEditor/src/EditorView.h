#pragma once
#include <Camera.h>
#include "EditorModule.h"
class EditorView : public EditorModule {
private:
	sa::Camera m_camera;
	RenderWindow* m_pWindow;

	glm::vec2 m_lastMousePos;
	float m_mouseSensitivity;
	float m_moveSpeed;

public:
	EditorView(sa::Engine* pEngine, RenderWindow* pWindow);
	~EditorView();

	// moves camera around scene according to input
	void update(float dt);

	virtual void onImGui() override;

	sa::Camera* getCamera();


};