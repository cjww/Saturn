#include "SceneHierarchy.h"

void SceneHierarchy::makePopups() {
	if (ImGui::BeginPopup("SceneHierarchyMenu")) {
		if (!m_isPopupMenuOpen)
			ImGui::CloseCurrentPopup();

		if (!m_hoveredEntity.isNull()) {	
			if (ImGui::MenuItem(("Delete " + m_hoveredEntity.getComponent<comp::Name>()->name).c_str())) {
				m_pEngine->publish<sa::editor_event::EntityDeselected>(m_hoveredEntity);
				m_hoveredEntity.destroy();
				ImGui::CloseCurrentPopup();
			}
			ImGui::Separator();
		}

		if (ImGui::BeginMenu("Add Entity")) {
			
			if (ImGui::MenuItem("Empty")) {
				sa::Entity entity = m_pEngine->getCurrentScene()->createEntity("Empty");
				entity.addComponent<comp::Transform>();
			}
			
			if (ImGui::MenuItem("Quad")) {
				sa::Entity entity = m_pEngine->getCurrentScene()->createEntity("Quad");
				entity.addComponent<comp::Transform>();
				entity.addComponent<comp::Model>()->modelID = sa::AssetManager::get().loadQuad();
			}
			
			if (ImGui::MenuItem("Box")) {
				sa::Entity entity = m_pEngine->getCurrentScene()->createEntity("Box");
				entity.addComponent<comp::Transform>();
				entity.addComponent<comp::Model>()->modelID = sa::AssetManager::get().loadBox();
			}

			ImGui::EndMenu();
		}

		ImGui::Text("Nr of entities: %d", m_pEngine->getCurrentScene()->getEntityCount());
		ImGui::EndPopup();
	}
}

SceneHierarchy::SceneHierarchy(sa::Engine* pEngine) : EditorModule(pEngine) {
	pEngine->on<sa::engine_event::SceneSet>([&](sa::engine_event::SceneSet& e, sa::Engine& engine) {
		m_hoveredEntity = {};
		m_selectedEntity = {};
		m_isPopupMenuOpen = false;
	});

	m_hoveredEntity = {};
	m_selectedEntity = {};
	m_isPopupMenuOpen = false;
}

void SceneHierarchy::elementEvents(const sa::Entity& e) {
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup)) {
		if (m_isPopupMenuOpen && m_hoveredEntity != e) {
			m_isPopupMenuOpen = false;
		}
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
	if (!e.hasParent()) {
		ImGuiStyle& style = ImGui::GetStyle();
		
		float oldpadding = style.TouchExtraPadding.y;
		style.TouchExtraPadding.y = 2;
		
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(1, 0));
		ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0, 0, 0, 0));
		ImGui::Separator();
		ImGui::PopStyleColor();
		ImGui::PopStyleVar();

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
	if (e == m_hoveredEntity && ImGui::IsPopupOpen("SceneHierarchyMenu")) {
		flags |= ImGuiTreeNodeFlags_Selected;
	}
	if (!e.hasChildren()) {
		flags |= ImGuiTreeNodeFlags_Leaf;
	}

	bool opened = ImGui::TreeNodeEx((e.getComponent<comp::Name>()->name + "##" + std::to_string((uint32_t)e)).c_str(), flags);
	if (ImGui::IsItemClicked()) {
		if (s) {
			m_pEngine->publish<sa::editor_event::EntityDeselected>(m_selectedEntity);
			m_selectedEntity = {};
		}
		else {
			m_selectedEntity = e;
			m_pEngine->publish<sa::editor_event::EntitySelected>(m_selectedEntity);
		}
	}
	
	elementEvents(e);
		
	if (opened) {

		m_pEngine->getCurrentScene()->getHierarchy().forEachDirectChild(e, std::bind(&SceneHierarchy::makeTree, this, std::placeholders::_1));
			
		ImGui::TreePop();
	}
	
}

SceneHierarchy::~SceneHierarchy() {

}

void SceneHierarchy::onImGui() {
	SA_PROFILE_FUNCTION();

	//ImGui::ShowStyleEditor();


	if (ImGui::Begin("Scene Hierarchy")) {

		//m_isPopupMenuOpen = ImGui::IsPopupOpen("SceneHierarchyMenu");

		if(!m_isPopupMenuOpen)
			m_hoveredEntity = {};


		makePopups();

		m_isPopupMenuOpen = ImGui::IsPopupOpen("SceneHierarchyMenu");


		sa::Scene* pScene = m_pEngine->getCurrentScene();

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
			m_isPopupMenuOpen = true;
		}
		
		
	}
	ImGui::End();


}

void SceneHierarchy::update(float dt) {

}
