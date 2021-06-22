#pragma once

#include <RenderWindow.hpp>
#include <Engine.h>

#include "EditorView.h"

class EngineEditor {
private:
	RenderWindow m_window;
	Engine m_engine;

	std::unique_ptr<EditorView> m_editorView;

public:
	EngineEditor();

	void run();
};