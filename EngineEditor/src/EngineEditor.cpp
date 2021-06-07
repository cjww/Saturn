#include "EngineEditor.h"

EngineEditor::EngineEditor()
	: m_window(1000, 600, "Saturn3")
{

}


void EngineEditor::run() {
	
	m_engine.setup(&m_window, "../setup.xml");



	while (m_window.isOpen()) {
		auto time = std::chrono::duration_cast<std::chrono::duration<double>>(m_engine.getCPUFrameTime());
		m_window.setWindowTitle(std::to_string(1.0 / time.count()) + " fps");
		float dt = static_cast<float>(time.count());
		
		m_engine.beginFrame();
		m_window.pollEvents();

		if (ImGui::Button("New Entity")) {
			EntityID entity = ECSCoordinator::get()->createEntity();
			ECSCoordinator::get()->addComponent<Transform>(entity);
			ECSCoordinator::get()->addComponent<Model>(entity)->modelID = ResourceManager::get()->loadQuad();
		}

		ImGui::ShowAboutWindow();

		m_engine.update();
		m_engine.draw();

	}
	m_engine.cleanup();
	
	
}
