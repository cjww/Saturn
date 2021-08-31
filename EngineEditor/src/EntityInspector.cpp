#include "EntityInspector.h"

void EntityInspector::makePopups() {
	if (ImGui::BeginPopup("Remove?")) {
		ImGui::Text("Remove?");
		if (ImGui::Button("Yes")) {
			ImGui::CloseCurrentPopup();
			ComponentType type = (ComponentType)ImGui::payload.data;
			ECSCoordinator::get()->removeComponent(type, m_currentEntity);
		}
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("Select Component")) {

		auto componentTypes = ECSCoordinator::get()->getAllComponentTypes();
		for (const auto& type : componentTypes) {
			if (ImGui::Button(ECSCoordinator::get()->getComponentName(type))) {
				ECSCoordinator::get()->addComponent(type, m_currentEntity);
				ImGui::CloseCurrentPopup();
			}
		}
		ImGui::EndPopup();
	}
}

EntityInspector::EntityInspector(sa::Engine* pEngine) : EditorModule(pEngine) {
	m_currentEntity = -1;
	m_removeComponent = -1;
}

EntityInspector::~EntityInspector() {

}

void EntityInspector::onImGui() {
	if (ImGui::Begin("Inspector")) {
		if (m_currentEntity != -1) {
			
			makePopups();
			
			char buffer[IMGUI_BUFFER_SIZE_SMALL];
			std::string entityName = ECSCoordinator::get()->getEntityName(m_currentEntity);
			strcpy_s(buffer, entityName.c_str());
			if (ImGui::InputText("Name##Entity", buffer, IMGUI_BUFFER_SIZE_SMALL, ImGuiInputTextFlags_EnterReturnsTrue)) {
				ECSCoordinator::get()->setEntityName(m_currentEntity, buffer);
			}
			if (ImGui::IsItemFocused()) {
				memcpy(buffer, entityName.c_str(), std::min((uint32_t)entityName.length(), IMGUI_BUFFER_SIZE_SMALL));
			}
			
			ImGui::Component<Transform>(m_currentEntity);
			ImGui::Component<Model>(m_currentEntity);
			ImGui::Component<Script>(m_currentEntity);
		
			
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

EntityID EntityInspector::getEntity() const {
	return m_currentEntity;
}

void EntityInspector::setEntity(EntityID id) {
	m_currentEntity = id;
}
