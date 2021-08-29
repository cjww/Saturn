#pragma once
#include <Camera.h>
#include "EditorModule.h"
#include <Tools\Math.h>

class EditorView : public EditorModule {
private:
	sa::Camera m_camera;
	RenderWindow* m_pWindow;

	bool m_isFocused;
	glm::vec2 m_lastMousePos;
	float m_mouseSensitivity;
	float m_moveSpeed;

	vr::Texture* m_pTexture;
	ImVec2 m_displayedSize;
	vr::SamplerPtr m_pSampler;

	Transform transform;

public:
	EditorView(sa::Engine* pEngine, RenderWindow* pWindow);
	virtual ~EditorView() override;

	// moves camera around scene according to input
	void update(float dt);

	virtual void onImGui() override;

	void imGuiDrawLine(glm::vec3 p1, glm::vec3 p2, const ImColor& color, float thickness = 1.0f);
	void imGuiDrawVector(glm::vec3 origin, glm::vec3 v, const ImColor& color, float thickness = 1.0f);
	void imGuiDrawVector(glm::vec3 v, const ImColor& color, float thickness = 1.0f);


	sa::Camera* getCamera();


};