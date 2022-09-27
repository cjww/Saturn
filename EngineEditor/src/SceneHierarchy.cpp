#include "SceneHierarchy.h"

void SceneHierarchy::makePopups() {
	if (ImGui::BeginPopup("SceneHierarchyMenu")) {
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

SceneHierarchy::SceneHierarchy(sa::Engine* pEngine) : EditorModule(pEngine) {
	
}

SceneHierarchy::~SceneHierarchy() {

}

void SceneHierarchy::tree(const sa::Entity& child) {
	bool s = m_selectedEntity == child;
	if (m_pEngine->getCurrentScene()->getHierarchy().hasChildren(child)) {
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
		if (s) flags |= ImGuiTreeNodeFlags_Selected;
		bool opened = ImGui::TreeNodeEx((child.getComponent<comp::Name>()->name + "##" + std::to_string((uint32_t)child)).c_str(), flags);
		if (ImGui::IsItemClicked()) {
			if (s) {
				m_pEngine->getCurrentScene()->publish<sa::editor_event::EntityDeselected>(m_selectedEntity);
				m_selectedEntity = {};
			}
			else {
				m_selectedEntity = child;
				m_pEngine->getCurrentScene()->publish<sa::editor_event::EntitySelected>(m_selectedEntity);
			}
		}
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup)) {
			m_hoveredEntity = child;
		}
		if (opened) {

			m_pEngine->getCurrentScene()->getHierarchy().forEachDirectChild(child, std::bind(&SceneHierarchy::tree, this, std::placeholders::_1));

			ImGui::TreePop();
		}
		return;
	}
	// leaf node
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetTreeNodeToLabelSpacing());
	if (ImGui::Selectable((child.getComponent<comp::Name>()->name + "##" + std::to_string((uint32_t)child)).c_str(), &s)) {
		if (s) {
			m_selectedEntity = child;
			m_pEngine->getCurrentScene()->publish<sa::editor_event::EntitySelected>(m_selectedEntity);
		}
		else {
			m_pEngine->getCurrentScene()->publish<sa::editor_event::EntityDeselected>(m_selectedEntity);
			m_selectedEntity = {};
		}
	}
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup)) {
		m_hoveredEntity = child;
	}
}

void SceneHierarchy::onImGui() {
	SA_PROFILE_FUNCTION();

	if (ImGui::Begin("Scene Hierarchy")) {

		sa::Scene* pScene = m_pEngine->getCurrentScene();

		if (!m_hoveredEntity.isNull()) {
			if (ImGui::BeginPopup("SceneHierarchyMenu")) {
				
				if (ImGui::Button("Delete")) {
					pScene->publish<sa::editor_event::EntityDeselected>(m_hoveredEntity);
					m_hoveredEntity.destroy();
					ImGui::CloseCurrentPopup();
				}
				ImGui::Separator();

				ImGui::EndPopup();
			}
		}
		makePopups();


		if (ImGui::BeginListBox("##Entities", ImGui::GetContentRegionAvail())) {
			pScene->forEach([&](sa::Entity e) {
				if (pScene->getHierarchy().hasParent(e))
					return;

				bool s = m_selectedEntity == e;
				if (pScene->getHierarchy().hasChildren(e)) {
					ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
					if (s) flags |= ImGuiTreeNodeFlags_Selected;
					bool opened = ImGui::TreeNodeEx((e.getComponent<comp::Name>()->name + "##" + std::to_string((uint32_t)e)).c_str(), flags);
					if (ImGui::IsItemClicked()) {
						if (s) {
							m_pEngine->getCurrentScene()->publish<sa::editor_event::EntityDeselected>(m_selectedEntity);
							m_selectedEntity = {};
						}
						else {
							m_selectedEntity = e;
							m_pEngine->getCurrentScene()->publish<sa::editor_event::EntitySelected>(m_selectedEntity);
						}
					}
					if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup)) {
						m_hoveredEntity = e;
					}
					if (opened) {

						m_pEngine->getCurrentScene()->getHierarchy().forEachDirectChild(e, std::bind(&SceneHierarchy::tree, this, std::placeholders::_1));

						ImGui::TreePop();
					}
					return;
				}
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetTreeNodeToLabelSpacing());
				if (ImGui::Selectable((e.getComponent<comp::Name>()->name + "##" + std::to_string((uint32_t)e)).c_str(), &s)) {
					if (s) {
						m_selectedEntity = e;
						m_pEngine->getCurrentScene()->publish<sa::editor_event::EntitySelected>(m_selectedEntity);
					}
					else {
						m_pEngine->getCurrentScene()->publish<sa::editor_event::EntityDeselected>(m_selectedEntity);
						m_selectedEntity = {};
					}
				}
				if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup)) {
					m_hoveredEntity = e;
				}
			});
	
			ImGui::EndListBox();
		}

		
		if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) 
			&& ImGui::IsMouseClicked(ImGuiMouseButton_Right)) 
		{		
			ImGui::OpenPopup("SceneHierarchyMenu");
		}

		if (!ImGui::IsPopupOpen("SceneHierarchyMenu")) {
			//itemMenu = false;
			m_hoveredEntity = {};
		}
		

		
	}
	ImGui::End();


}

void SceneHierarchy::update(float dt) {

}
