#pragma once
#include <RenderWindow.hpp>

#include <Renderer.hpp>


class IRenderTechnique {
protected:
	bool m_useImGui;

public:
	virtual void init(RenderWindow* pWindow, bool setupImGui = false) = 0;
	virtual void cleanup() = 0;

	bool isUsingImGui() const;
	virtual void beginFrameImGUI() = 0;

	virtual void draw() = 0;
};