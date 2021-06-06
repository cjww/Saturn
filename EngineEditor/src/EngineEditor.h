#pragma once

#include <RenderWindow.hpp>
#include <Engine.h>

class EngineEditor {
private:
	RenderWindow m_window;
	Engine m_engine;

public:
	EngineEditor();

	void run();
};