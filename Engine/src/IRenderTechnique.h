#pragma once
#include <RenderWindow.hpp>

class IRenderTechnique {
protected:

public:
	virtual void init(RenderWindow* pWindow) = 0;
	virtual void cleanup() = 0;

	virtual void draw() = 0;
};