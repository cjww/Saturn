#include "EngineEditor.h"

#include <Tools\Clock.h>

void EngineEditor::onImGui() {
	

	ImGuiID viewPortDockSpaceID = ImGui::DockSpaceOverViewport();
	if (ImGui::BeginMainMenuBar()) {
		ImGui::Text("Saturn 3");
	}
	ImGui::EndMainMenuBar();

	for (auto& module : m_editorModules) {
		module->onImGui();
	}

	ImGui::ShowDemoWindow();
}

EngineEditor::EngineEditor()
	: m_window(1500, 800, "Saturn3")
{

}

void EngineEditor::openProject(const std::string& projectPath) {
	
	sa::Scene& scene = m_engine.setup(&m_window, "../setup.xml");

	m_editorModules.push_back(std::make_unique<EditorView>(&m_engine, &m_window));
	EditorView* editorView = static_cast<EditorView*>(m_editorModules.back().get());

	m_engine.getCurrentScene()->addActiveCamera(editorView->getCamera());

		
	m_editorModules.push_back(std::make_unique<EntityInspector>(&m_engine));
	
	m_editorModules.push_back(std::make_unique<SceneView>(&m_engine));
	
	sa::Entity e = scene.createEntity("Quad");
	e.addComponent<comp::Transform>();
	e.addComponent<comp::Model>()->modelID = sa::ResourceManager::get().loadQuad();
	
	m_engine.createSystemScript("test.lua");

	sa::Clock clock;
	while (m_window.isOpen()) {
		m_window.pollEvents();

		m_engine.recordImGui();

		onImGui();

		
		float dt = clock.restart();
		m_engine.update(dt);
		m_window.setWindowTitle(std::to_string(1.0 / dt) + " fps");

		for (auto& module : m_editorModules) {
			module->update(dt);
		}
		m_engine.draw();

	}

	m_editorModules.clear();
	m_engine.cleanup();
	
}
