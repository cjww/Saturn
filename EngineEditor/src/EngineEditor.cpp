#include "EngineEditor.h"

#include <Tools\Clock.h>

#include "TestLayer.h"

namespace sa {

	void EngineEditor::onAttach(sa::Engine& engine, sa::RenderWindow& renderWindow) {

		m_editorModules.push_back(std::make_unique<SceneView>(&engine, &renderWindow));

		m_editorModules.push_back(std::make_unique<EntityInspector>(&engine));

		m_editorModules.push_back(std::make_unique<SceneHierarchy>(&engine));

		m_editorModules.push_back(std::make_unique<LuaConsole>(&engine));

		getApp()->pushLayer(new TestLayer);

	}

	void EngineEditor::onDetach() {
		m_editorModules.clear();
	}

	void EngineEditor::onImGuiRender() {
		ImGuiID viewPortDockSpaceID = ImGui::DockSpaceOverViewport();
		if (ImGui::BeginMainMenuBar()) {
			ImGui::Text("Saturn 3");
		}
		ImGui::EndMainMenuBar();

		for (auto& module : m_editorModules) {
			module->onImGui();
		}
	}

	void EngineEditor::onUpdate(float dt) {
		for (auto& module : m_editorModules) {
			module->update(dt);
		}
	}
}