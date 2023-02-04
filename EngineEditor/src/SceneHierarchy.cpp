#include "SceneHierarchy.h"

void SceneHierarchy::makePopups() {
	if (ImGui::BeginPopup("SceneHierarchyMenu")) {
		if (!m_isPopupMenuOpen)
			ImGui::CloseCurrentPopup();

		if (!m_hoveredEntity.isNull()) {	
			//ImGui::Text(m_hoveredEntity.getComponent<comp::Name>()->name.c_str());
			if (ImGui::MenuItem("Delete")) {
				m_pEngine->publish<sa::editor_event::EntityDeselected>(m_hoveredEntity);
				m_hoveredEntity.destroy();
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::MenuItem("Duplicate")) {
				sa::Entity clone = m_hoveredEntity.clone();
				m_pEngine->publish<sa::editor_event::EntitySelected>(clone);

				ImGui::CloseCurrentPopup();
			}
			

			ImGui::Separator();
		}

		if (ImGui::BeginMenu("Add Entity")) {
			
			if (ImGui::MenuItem("Empty")) {
				sa::Entity entity = m_pEngine->getCurrentScene()->createEntity("Empty");
				entity.addComponent<comp::Transform>();
				m_pEngine->publish<sa::editor_event::EntitySelected>(entity);
			}
			
			if (ImGui::MenuItem("Quad")) {
				sa::Entity entity = m_pEngine->getCurrentScene()->createEntity("Quad");
				entity.addComponent<comp::Transform>();
				entity.addComponent<comp::Model>()->modelID = sa::AssetManager::get().loadQuad();
				m_pEngine->publish<sa::editor_event::EntitySelected>(entity);
			}
			
			if (ImGui::MenuItem("Box")) {
				sa::Entity entity = m_pEngine->getCurrentScene()->createEntity("Box");
				entity.addComponent<comp::Transform>();
				entity.addComponent<comp::Model>()->modelID = sa::AssetManager::get().loadBox();
				m_pEngine->publish<sa::editor_event::EntitySelected>(entity);
			}

			if (ImGui::MenuItem("Camera")) {
				sa::Entity entity = m_pEngine->getCurrentScene()->createEntity("Camera");
				entity.addComponent<comp::Transform>();
				entity.addComponent<comp::Camera>();
				m_pEngine->publish<sa::editor_event::EntitySelected>(entity);
			}

			if (ImGui::MenuItem("Light")) {
				sa::Entity entity = m_pEngine->getCurrentScene()->createEntity("Light");
				entity.addComponent<comp::Transform>();
				entity.addComponent<comp::Light>();
				m_pEngine->publish<sa::editor_event::EntitySelected>(entity);
			}

			ImGui::EndMenu();
		}

		ImGui::Text("Nr of entities: %d", m_pEngine->getCurrentScene()->getEntityCount());
		ImGui::EndPopup();
	}
}

SceneHierarchy::SceneHierarchy(sa::Engine* pEngine, sa::EngineEditor* pEditor) 
	: EditorModule(pEngine, pEditor, "Scene Hierarchy", false) {
	pEngine->on<sa::engine_event::SceneSet>([&](sa::engine_event::SceneSet& e, sa::Engine& engine) {
		m_hoveredEntity = {};
		m_selectedEntity = {};
		m_isPopupMenuOpen = false;
	});

	pEngine->on<sa::editor_event::EntitySelected>([&](const sa::editor_event::EntitySelected& e, sa::Engine&) {
		m_selectedEntity = e.entity;
	});

	pEngine->on<sa::editor_event::EntityDeselected>([&](const sa::editor_event::EntityDeselected&, sa::Engine&) {
		m_selectedEntity = {};
	});

	m_hoveredEntity = {};
	m_selectedEntity = {};
	m_isPopupMenuOpen = false;
}

void SceneHierarchy::elementEvents(const sa::Entity& e) {
	static sa::Entity payload;
	if (ImGui::BeginDragDropSource()) {
		ImGui::SetDragDropPayload("Entity", &payload, sizeof(sa::Entity));
		ImGui::SetTooltip(payload.getComponent<comp::Name>()->name.c_str());
		ImGui::EndDragDropSource();
	}
	else if (ImGui::BeginDragDropTarget()) {
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity");
		if (payload && payload->IsDelivery()) {
			sa::Entity* entityPayload = (sa::Entity*)payload->Data;
			m_parentChanges.push(std::make_pair(*entityPayload, e));
		}

		ImGui::EndDragDropTarget();
	}
	else if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup)) {
		if (m_isPopupMenuOpen && m_hoveredEntity != e) {
			m_isPopupMenuOpen = false;
		}
		m_hoveredEntity = e;
		payload = m_hoveredEntity;
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
	if (e == m_hoveredEntity && m_isPopupMenuOpen) {
		flags |= ImGuiTreeNodeFlags_Selected;
	}
	if (!e.hasChildren()) {
		flags |= ImGuiTreeNodeFlags_Leaf;
	}

	bool opened = ImGui::TreeNodeEx((e.getComponent<comp::Name>()->name + "##" + std::to_string((uint32_t)e)).c_str(), flags);
	if (ImGui::IsItemClicked()) {
		if (s) {
			m_pEngine->publish<sa::editor_event::EntityDeselected>(e);
		}
		else {
			m_pEngine->publish<sa::editor_event::EntitySelected>(e);
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


	if (ImGui::Begin(m_name)) {

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
