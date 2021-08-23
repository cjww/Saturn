#include "EngineEditor.h"

void EngineEditor::onImGui() {
	for (auto& module : m_editorModules) {
		module->onImGui();
	}

	//const ImU32 col = ImColor(1, 0, 0, 1);
	//ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(0, 0), 50, col, 8);
	
	//ImGui::GetWindowDrawList()->PushClipRectFullScreen();
	//ImGui::GetWindowDrawList()->AddLine(ImVec2(0, 0), ImVec2(10, 10), col, 1.f);
	//ImGui::GetWindowDrawList()->PopClipRect();

	ImGui::Begin("Entities");
	if (ImGui::Button("New Entity")) {
		EntityID entity = ECSCoordinator::get()->createEntity();
		Transform* transform = ECSCoordinator::get()->addComponent<Transform>(entity);
		ECSCoordinator::get()->addComponent<Model>(entity)->modelID = sa::ResourceManager::get()->loadQuad();
		transform->position = glm::vec3(0, 0, -(rand() % 10));
	}
	ImGui::Text("Nr of entities: %d", ECSCoordinator::get()->getEntityCount());
	ImGui::End();
	/*
	*/

}

EngineEditor::EngineEditor()
	: m_window(1500, 800, "Saturn3")
{

}


void EngineEditor::run() {
	
	m_engine.setup(&m_window, "../setup.xml");

	m_editorModules.push_back(std::make_unique<EditorView>(&m_engine, &m_window));
	m_engine.addActiveCamera(static_cast<EditorView*>(m_editorModules.back().get())->getCamera());

	/*
	*/
	vr::Image img("../Box.png");
	vr::Texture* tex = vr::Renderer::get()->createTexture2D(img);
	vr::SamplerPtr sampler = vr::Renderer::get()->createSampler(VK_FILTER_NEAREST);

	EntityID entity = ECSCoordinator::get()->createEntity();
	ECSCoordinator::get()->addComponent<Transform>(entity);
	ECSCoordinator::get()->addComponent<Model>(entity)->modelID = sa::ResourceManager::get()->loadQuad();


	srand(time(NULL));
	while (m_window.isOpen()) {
		auto time = std::chrono::duration_cast<std::chrono::duration<double>>(m_engine.getCPUFrameTime());
		m_window.setWindowTitle(std::to_string(1.0 / time.count()) + " fps");
		float dt = static_cast<float>(time.count());

		m_window.pollEvents();
		m_engine.recordImGui();

		//onImGui();
		ImGui::ShowMetricsWindow();
		/*
		ImGui::Begin("Test");
		vr::Renderer::get()->imGuiImage(tex, sampler);
		ImGui::End();
		*/
		
		
		m_engine.update();
		for (auto& module : m_editorModules) {
			module->update(dt);
		}
		m_engine.draw();

	}

	sampler.reset();

	m_engine.cleanup();
	
	
}
