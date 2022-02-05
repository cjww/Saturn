#include "EntityInspector.h"

void EntityInspector::makePopups() {
	if (ImGui::BeginPopup("Remove?")) {
		ImGui::Text("Remove?");
		if (ImGui::Button("Yes")) {
			ImGui::CloseCurrentPopup();
			m_selectedEntity.removeComponent(ImGui::payload.type);
		}
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("Select Component")) {

		for (auto type : entt::resolve()) {
			sa::ComponentType compType(type);
			
			if (ImGui::Button(compType.getName().c_str())) {
				m_selectedEntity.addComponent(compType);
				ImGui::CloseCurrentPopup();
			}
		}
		ImGui::EndPopup();
	}
}

EntityInspector::EntityInspector(sa::Engine* pEngine) : EditorModule(pEngine) {
	m_selectedEntity = {};
}

EntityInspector::~EntityInspector() {

}

void EntityInspector::onImGui() {
	if (ImGui::Begin("Inspector")) {

		m_pEngine->getCurrentScene()->on<event::EntitySelected>([&](const event::EntitySelected& e, sa::Scene&) {
			m_selectedEntity = e.entity;
		});

		m_pEngine->getCurrentScene()->on<event::EntityDeselected>([&](const event::EntityDeselected&, sa::Scene&) {
			m_selectedEntity = {};
		});

		if (m_selectedEntity) {
			
			makePopups();
			
			char buffer[IMGUI_BUFFER_SIZE_TINY];
			//sa::Scene* pScene= m_pEngine->getCurrentScene();
			comp::Name* nameComp = m_selectedEntity.getComponent<comp::Name>();

			
			strcpy_s(buffer, nameComp->name.c_str());
			if (ImGui::InputText("Name##Entity", buffer, IMGUI_BUFFER_SIZE_TINY, ImGuiInputTextFlags_EnterReturnsTrue)) {
				nameComp->name = buffer;
			}
			
			ImGui::Component<comp::Transform>(m_selectedEntity);
			ImGui::Component<comp::Model>(m_selectedEntity);
			ImGui::Component<comp::Script>(m_selectedEntity);
			
			
			
			ImGui::Separator();
			ImGui::Spacing();
			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() / 3);
			
			if (ImGui::Button("Add Component +", ImVec2(-ImGui::GetWindowContentRegionWidth() / 3.f, 0))) {
				ImGui::OpenPopup("Select Component");
			}
		}

	}
	ImGui::End();
}

void EntityInspector::update(float dt) {

}

sa::Entity EntityInspector::getEntity() const {
	return m_selectedEntity;
}

void EntityInspector::setEntity(const sa::Entity& entity) {
	m_selectedEntity = entity;
}