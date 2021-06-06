#include "EngineEditor.h"

EngineEditor::EngineEditor()
	: m_window(1000, 600, "Saturn3")
{

}

void EngineEditor::run() {

	m_engine.setup(&m_window, "../setup.xml");
	EntityID entity = ECSCoordinator::get()->createEntity();
	ECSCoordinator::get()->addComponent<Transform>(entity);
	ECSCoordinator::get()->addComponent<Model>(entity)->modelID = ResourceManager::get()->loadQuad();

	while (m_window.isOpen()) {
		m_window.pollEvents();
		auto time = std::chrono::duration_cast<std::chrono::duration<double>>(m_engine.getCPUFrameTime());
		m_window.setWindowTitle(std::to_string(1.0 / time.count()) + " fps");
		float dt = static_cast<float>(time.count());

		m_engine.update();
		m_engine.draw();

	}

	m_engine.cleanup();
}
