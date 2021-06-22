#pragma once
#include <Camera.h>

class EditorView {
private:
	Camera m_camera;

public:
	EditorView(const RenderWindow* pWindow);
	~EditorView();

	Camera* getCamera();

};