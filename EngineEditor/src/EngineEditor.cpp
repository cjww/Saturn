#include "EngineEditor.h"

void EngineEditor::onImGui() {
	

	ImGuiID viewPortDockSpaceID = ImGui::DockSpaceOverViewport();
	if (ImGui::BeginMainMenuBar()) {
		ImGui::Text("Saturn 3");
	}
	ImGui::EndMainMenuBar();

	for (auto& module : m_editorModules) {
		module->onImGui();
	}

}

EngineEditor::EngineEditor()
	: m_window(1500, 800, "Saturn3")
{

}

void EngineEditor::openProject(const std::string& projectPath) {
	/*
	sa::registerComponentType<comp::Name>();
	sa::registerComponentType<comp::Transform>();
	sa::registerComponentType<comp::Model>();
	sa::registerComponentType<comp::Script>();
	*/

	m_engine.setup(&m_window, "../setup.xml");

	if (!m_engine.getCurrentScene()) {
		m_engine.setScene(m_engine.getScene("SaturnScene"));
	}


	m_editorModules.push_back(std::make_unique<EditorView>(&m_engine, &m_window));
	EditorView* editorView = static_cast<EditorView*>(m_editorModules.back().get());

	m_engine.getCurrentScene()->addActiveCamera(editorView->getCamera());

		
	m_editorModules.push_back(std::make_unique<EntityInspector>(&m_engine));
	
	m_editorModules.push_back(std::make_unique<SceneView>(&m_engine));
	
	sa::Entity e = m_engine.getCurrentScene()->createEntity("Quad");
	e.addComponent<comp::Transform>();
	e.addComponent<comp::Model>()->modelID = sa::ResourceManager::get().loadQuad();
	

	srand(time(NULL));
	while (m_window.isOpen()) {
		auto time = std::chrono::duration_cast<std::chrono::duration<double>>(m_engine.getCPUFrameTime());
		m_window.setWindowTitle(std::to_string(1.0 / time.count()) + " fps");
		float dt = static_cast<float>(time.count());

		m_window.pollEvents();
		m_engine.recordImGui();


		onImGui();
		
		
		m_engine.update(dt);
		for (auto& module : m_editorModules) {
			module->update(dt);
		}
		m_engine.draw();

	}

	m_editorModules.clear();
	m_engine.cleanup();
	
}
