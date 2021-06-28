#include "EngineEditor.h"

EngineEditor::EngineEditor()
	: m_window(1500, 800, "Saturn3")
{

}


void EngineEditor::run() {
	
	m_engine.setup(&m_window, "../setup.xml");

	m_editorView = std::make_unique<EditorView>(&m_window);
	m_engine.addActiveCamera(m_editorView->getCamera());


	srand(time(NULL));
	while (m_window.isOpen()) {
		auto time = std::chrono::duration_cast<std::chrono::duration<double>>(m_engine.getCPUFrameTime());
		m_window.setWindowTitle(std::to_string(1.0 / time.count()) + " fps");
		float dt = static_cast<float>(time.count());
		
		m_window.pollEvents();
		m_engine.recordImGui();
		ImGui::Begin("Entities");
		if (ImGui::Button("New Entity")) {			
			EntityID entity = ECSCoordinator::get()->createEntity();
			Transform* transform = ECSCoordinator::get()->addComponent<Transform>(entity);
			ECSCoordinator::get()->addComponent<Model>(entity)->modelID = ResourceManager::get()->loadQuad();
			transform->position = glm::vec3(0, 0, -(rand() % 10));
		}
		ImGui::Text("Nr of entities: %d", ECSCoordinator::get()->getEntityCount());
		ImGui::End();
		
		m_engine.update();
		m_editorView->update(dt);

		m_engine.draw();

	}
	m_engine.cleanup();
	
	
}
