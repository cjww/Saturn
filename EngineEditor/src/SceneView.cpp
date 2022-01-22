#include "SceneView.h"

void SceneView::makePopups() {
	if (ImGui::BeginPopup("SceneViewMenu")) {
		if (ImGui::BeginMenu("Add Entity")) {
			
			if (ImGui::Button("Empty")) {
				Entity entity = m_pEngine->getCurrentScene()->createEntity("Empty");
				entity.addComponent<comp::Transform>();
			}
			
			if (ImGui::Button("Quad")) {
				Entity entity = m_pEngine->getCurrentScene()->createEntity("Quad");
				entity.addComponent<comp::Transform>();
				entity.addComponent<comp::Model>()->modelID = sa::ResourceManager::get()->loadQuad();
			}
			
			ImGui::EndMenu();
		}

		ImGui::Text("Nr of entities: %d", m_pRegistry->size());
		ImGui::EndPopup();
	}
}

SceneView::SceneView(sa::Engine* pEngine, EntityInspector* pInspector, EditorView* pView) : EditorModule(pEngine) {
	m_pInspector = pInspector;
	m_pView = pView;
	m_pRegistry = nullptr;
	if (pEngine->getCurrentScene()) {
		m_pRegistry = &pEngine->getCurrentScene()->getRegistry();
	}
}

SceneView::~SceneView() {

}

void SceneView::onImGui() {
	if (ImGui::Begin("Scene View")) {

		static bool itemMenu = false;
		static entt::entity hovered = entt::null;
		if (itemMenu) {
			if (ImGui::BeginPopup("SceneViewMenu")) {
				
				if (ImGui::Button("Delete")) {
					if (hovered != entt::null) {
						//ECSCoordinator::get()->destroyEntity(hovered);

						m_pInspector->setEntity(entt::null);
						ImGui::CloseCurrentPopup();
					}
				}
				ImGui::Separator();

				ImGui::EndPopup();
			}
		}
		makePopups();


		ImGui::BeginListBox("##Entities");

		static entt::entity selected = entt::null;

		m_pRegistry->each([&](entt::entity e) {
			bool s = selected == e;
			if (ImGui::Selectable((std::string("Entity") + "##" + std::to_string((int)e)).c_str(), &s)) {
				if (s) selected = e;
				else selected = entt::null;
				m_pInspector->setEntity(selected);
				m_pView->setEntity(selected);
			}
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup)) {
				hovered = e;
				itemMenu = true;
			}
		});
	
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
