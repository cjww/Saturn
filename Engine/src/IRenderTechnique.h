#pragma once
#include <RenderWindow.hpp>

#include <Renderer.hpp>
#include "Camera.h"

class IRenderTechnique {
protected:
	bool m_useImGui;

	std::set<Camera*> m_activeCameras;

public:
	virtual void init(RenderWindow* pWindow, bool setupImGui = false) = 0;
	virtual void cleanup() = 0;

	bool isUsingImGui() const;
	virtual void beginFrameImGUI() = 0;

	virtual void draw() = 0;

	void addCamera(Camera* camera);
	void removeCamera(Camera* camera);

};