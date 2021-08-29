#include "SceneView.h"

SceneView::SceneView(sa::Engine* pEngine) : EditorModule(pEngine){

}

SceneView::~SceneView() {

}

void SceneView::onImGui() {
	if (ImGui::Begin("Scene View")) {
		ImGui::BeginListBox("Entities");
		auto entities = ECSCoordinator::get()->getActiveEntities();
		for (const auto& e : entities) {
			ImGui::Text("Entity: %d", e);
		}

		ImGui::EndListBox();

		if (ImGui::Button("New Entity")) {
			EntityID entity = ECSCoordinator::get()->createEntity();
			Transform* transform = ECSCoordinator::get()->addComponent<Transform>(entity);
			ECSCoordinator::get()->addComponent<Model>(entity)->modelID = sa::ResourceManager::get()->loadQuad();
			transform->position = glm::vec3(0, 0, -(rand() % 10));
		}
		ImGui::Text("Nr of entities: %d", ECSCoordinator::get()->getEntityCount());
		ImGui::End();
	}

}

void SceneView::update(float dt) {

}
