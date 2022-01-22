#pragma once

#include <RenderWindow.hpp>
#include <Engine.h>

#include "EditorView.h"
#include "SceneView.h"
#include "EntityInspector.h"

class EngineEditor {
private:
	RenderWindow m_window;
	sa::Engine m_engine;

	std::vector<std::unique_ptr<EditorModule>> m_editorModules;

	void onImGui();

public:
	EngineEditor();

	void openProject(const std::string& projectPath);
};