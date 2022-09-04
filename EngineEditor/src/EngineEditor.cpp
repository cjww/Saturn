#include "EngineEditor.h"

#include <Tools\Clock.h>

#include "TestLayer.h"

namespace sa {

	void EngineEditor::onAttach(sa::Engine& engine, sa::RenderWindow& renderWindow) {

		m_editorModules.push_back(std::make_unique<EditorView>(&engine, &renderWindow));
		EditorView* editorView = static_cast<EditorView*>(m_editorModules.back().get());

		engine.getCurrentScene()->addActiveCamera(editorView->getCamera());
		/*
		//Testing multiple viewports
		editorView->getCamera()->setViewport({ { 0, 0 }, {renderWindow.getCurrentExtent().width / 2, renderWindow.getCurrentExtent().height } });

		testCamera.setPosition({ 0, 0, 5 });
		testCamera.lookAt({ 0, 0, 0 });
		testCamera.setViewport({ { (int)renderWindow.getCurrentExtent().width / 2, 0 }, { renderWindow.getCurrentExtent().width / 2, renderWindow.getCurrentExtent().height } });
		engine.getCurrentScene()->addActiveCamera(&testCamera);
		*/


		m_editorModules.push_back(std::make_unique<EntityInspector>(&engine));

		m_editorModules.push_back(std::make_unique<SceneView>(&engine));

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