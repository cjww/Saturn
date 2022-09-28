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

void SceneHierarchy::elementEvents(const sa::Entity& e) {
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup)) {
		m_hoveredEntity = e;
	}
	if (ImGui::BeginDragDropSource()) {
		ImGui::SetDragDropPayload("Entity", &m_hoveredEntity, sizeof(sa::Entity));
		ImGui::SetTooltip(m_hoveredEntity.getComponent<comp::Name>()->name.c_str());
		ImGui::EndDragDropSource();
	}
	if (ImGui::BeginDragDropTarget()) {
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity");
		if (payload && payload->IsDelivery()) {
			sa::Entity* entityPayload = (sa::Entity*)payload->Data;
			m_parentChanges.push(std::make_pair(*entityPayload, e));
		}

		ImGui::EndDragDropTarget();
	}
}

void SceneHierarchy::makeTree(sa::Entity e) {
	bool s = m_selectedEntity == e;
	// Tree node
	if (m_pEngine->getCurrentScene()->getHierarchy().hasChildren(e)) {
		if (!m_pEngine->getCurrentScene()->getHierarchy().hasParent(e)) {
			ImGuiStyle& style = ImGui::GetStyle();
			float oldpadding = style.TouchExtraPadding.y;
			style.TouchExtraPadding.y = 2;
			ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0, 0, 0, 0));
			ImGui::Separator();
			ImGui::PopStyleColor();
			style.TouchExtraPadding.y = oldpadding;
			if (ImGui::BeginDragDropTarget()) {
				const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity");
				if (payload && payload->IsDelivery()) {
					sa::Entity* payloadEntity = (sa::Entity*)payload->Data;
					m_parentChanges.push(std::make_pair(*payloadEntity, sa::Entity()));
				}
				ImGui::EndDragDropTarget();
			}
		}

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
		elementEvents(e);
		
		if (opened) {

			m_pEngine->getCurrentScene()->getHierarchy().forEachDirectChild(e, std::bind(&SceneHierarchy::makeTree, this, std::placeholders::_1));
			
			ImGui::TreePop();
		}
		return;
	}
	// Leaf node
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
	elementEvents(e);
}

SceneHierarchy::~SceneHierarchy() {

}

void SceneHierarchy::onImGui() {
	SA_PROFILE_FUNCTION();

	ImGui::ShowStyleEditor();

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
				// only make trees from roots
				makeTree(e);
			});

			while (!m_parentChanges.empty()) {
				auto& pair = m_parentChanges.front();
				if (!pair.second.isNull())
					pair.first.setParent(pair.second);
				else
					pair.first.orphan();
				m_parentChanges.pop();
			}
	
			ImGui::EndListBox();
		}
		
		if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) 
			&& ImGui::IsMouseClicked(ImGuiMouseButton_Right)) 
		{		
			ImGui::OpenPopup("SceneHierarchyMenu");
		}
		
	}
	ImGui::End();


}

void SceneHierarchy::update(float dt) {

}
