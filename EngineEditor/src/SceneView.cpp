#include "SceneView.h"

void SceneView::makePopups() {
	if (ImGui::BeginPopup("SceneViewMenu")) {
		if (ImGui::BeginMenu("Add Entity")) {

			if (ImGui::Button("Empty")) {
				EntityID entity = ECSCoordinator::get()->createEntity("Empty");
				ECSCoordinator::get()->addComponent<Transform>(entity);
			}
			if (ImGui::Button("Quad")) {
				EntityID entity = ECSCoordinator::get()->createEntity("Quad");
				Transform* transform = ECSCoordinator::get()->addComponent<Transform>(entity);
				ECSCoordinator::get()->addComponent<Model>(entity)->modelID = sa::ResourceManager::get()->loadQuad();
			}
			ImGui::EndMenu();
		}

		ImGui::Text("Nr of entities: %d", ECSCoordinator::get()->getEntityCount());
		ImGui::EndPopup();
	}
}

SceneView::SceneView(sa::Engine* pEngine, EntityInspector* pInspector) : EditorModule(pEngine) {
	m_pInspector = pInspector;
}

SceneView::~SceneView() {

}

void SceneView::onImGui() {
	if (ImGui::Begin("Scene View")) {

		static bool itemMenu = false;
		static EntityID hovered = -1;
		if (itemMenu) {
			if (ImGui::BeginPopup("SceneViewMenu")) {
				
				if (ImGui::Button("Delete")) {
					if (hovered != -1) {
						ECSCoordinator::get()->destroyEntity(hovered);
						m_pInspector->setEntity(-1);
						ImGui::CloseCurrentPopup();
					}
				}
				ImGui::Separator();

				ImGui::EndPopup();
			}
		}
		makePopups();


		ImGui::BeginListBox("##Entities");

		auto entities = ECSCoordinator::get()->getActiveEntities();
		static EntityID selected = -1;
		
		for (const auto& e : entities) {
			bool s = selected == e;
			if (ImGui::Selectable((ECSCoordinator::get()->getEntityName(e) + "##" + std::to_string(e)).c_str(), &s)) {
				if (s) selected = e;
				else selected = -1;
				m_pInspector->setEntity(selected);
			}
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup)) {
				hovered = e;
				itemMenu = true;
			}
		}

		ImGui::EndListBox();

		
		if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) 
			&& ImGui::IsMouseClicked(ImGuiMouseButton_Right)) 
		{		
			ImGui::OpenPopup("SceneViewMenu");
		}

		if (!ImGui::IsPopupOpen("SceneViewMenu")) {
			itemMenu = false;
		}
		

		
	}
	ImGui::End();


}

void SceneView::update(float dt) {

}
