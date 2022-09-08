#include "SceneView.h"

void SceneView::makePopups() {
	if (ImGui::BeginPopup("SceneViewMenu")) {
		if (ImGui::BeginMenu("Add Entity")) {
			
			if (ImGui::Button("Empty")) {
				sa::Entity entity = m_pEngine->getCurrentScene()->createEntity("Empty");
				entity.addComponent<comp::Transform>();
			}
			
			if (ImGui::Button("Quad")) {
				sa::Entity entity = m_pEngine->getCurrentScene()->createEntity("Quad");
				entity.addComponent<comp::Transform>();
				entity.addComponent<comp::Model>()->modelID = sa::AssetManager::get().loadQuad();
			}
			
			ImGui::EndMenu();
		}

		ImGui::Text("Nr of entities: %d", m_pEngine->getCurrentScene()->getEntityCount());
		ImGui::EndPopup();
	}
}

SceneView::SceneView(sa::Engine* pEngine) : EditorModule(pEngine) {
	
}

SceneView::~SceneView() {

}

void SceneView::onImGui() {
	SA_PROFILE_FUNCTION();
	if (ImGui::Begin("Scene View")) {

		sa::Scene* pScene = m_pEngine->getCurrentScene();

		static bool itemMenu = false;
		static sa::Entity hovered;
		if (itemMenu) {
			if (ImGui::BeginPopup("SceneViewMenu")) {
				
				if (ImGui::Button("Delete")) {
					if (hovered) {
						pScene->publish<event::EntityDeselected>(hovered);
						pScene->destroyEntity(hovered);
						ImGui::CloseCurrentPopup();
					}
				}
				ImGui::Separator();

				ImGui::EndPopup();
			}
		}
		makePopups();


		if (ImGui::BeginListBox("##Entities", ImGui::GetContentRegionAvail())) {
			static sa::Entity selected;
			pScene->forEach([&](sa::Entity e) {
				bool s = selected == e;

				if (ImGui::Selectable((e.getComponent<comp::Name>()->name + "##" + std::to_string((uint32_t)e)).c_str(), &s)) {
					if (s) {
						selected = e;
						m_pEngine->getCurrentScene()->publish<event::EntitySelected>(selected);
					}
					else {
						selected = {};
						m_pEngine->getCurrentScene()->publish<event::EntityDeselected>(selected);
					}
				}
				if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup)) {
					hovered = e;
					itemMenu = true;
				}
			});
	
			ImGui::EndListBox();
		}

		
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
