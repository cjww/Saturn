#include "EntityInspector.h"

void EntityInspector::makePopups() {
	if (ImGui::BeginPopup("Remove?")) {
		ImGui::Text("Remove?");
		if (ImGui::Button("Yes")) {
			ImGui::CloseCurrentPopup();
			entt::type_info type = ImGui::payload.type;

			using namespace entt::literals;
			//ECSCoordinator::get()->removeComponent(type, m_currentEntity);
			auto mt_type = entt::resolve(type);
			mt_type.func("remove"_hs).invoke({}, *m_registry, m_currentEntity);
		}
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("Select Component")) {

		auto componentTypes = ECSCoordinator::get()->getAllComponentTypes();
		for (const auto& type : componentTypes) {
			if (ImGui::Button(ECSCoordinator::get()->getComponentName(type))) {
				entt::type_info type = ImGui::payload.type;

				using namespace entt::literals;
				//ECSCoordinator::get()->removeComponent(type, m_currentEntity);
				auto mt_type = entt::resolve(type);
				mt_type.func("add"_hs).invoke({}, *m_registry, m_currentEntity);

				ImGui::CloseCurrentPopup();
			}
		}
		ImGui::EndPopup();
	}
}

EntityInspector::EntityInspector(sa::Engine* pEngine) : EditorModule(pEngine) {
	m_currentEntity = entt::null;
	m_removeComponent = -1;
	m_registry = nullptr;

	using namespace entt::literals;
	entt::meta<comp::Transform>().func<&get<comp::Transform>, entt::as_ref_t>("get"_hs);
	entt::meta<comp::Transform>().func<&remove<comp::Transform>>("remove"_hs);
	entt::meta<comp::Transform>().func<&add<comp::Transform>, entt::as_ref_t>("add"_hs);



}

EntityInspector::~EntityInspector() {

}

void EntityInspector::onImGui() {
	if (ImGui::Begin("Inspector")) {
		if (m_currentEntity != entt::null) {
			
			makePopups();
			
			char buffer[IMGUI_BUFFER_SIZE_SMALL];
			std::string entityName = std::to_string((int)m_currentEntity);
			entt::registry& reg = m_pEngine->getCurrentScene()->getRegistry();
			comp::Name* nameComp = reg.try_get<comp::Name>(m_currentEntity);
			if (nameComp)
				entityName = nameComp->name;

			strcpy_s(buffer, entityName.c_str());
			if (ImGui::InputText("Name##Entity", buffer, IMGUI_BUFFER_SIZE_SMALL, ImGuiInputTextFlags_EnterReturnsTrue)) {
				if (!nameComp)
					reg.emplace<comp::Name>(m_currentEntity, buffer);
			}
			if (ImGui::IsItemFocused()) {
				memcpy(buffer, entityName.c_str(), std::min((uint32_t)entityName.length(), IMGUI_BUFFER_SIZE_SMALL));
			}
			
			ImGui::Component<comp::Transform>(reg, m_currentEntity);
			ImGui::Component<comp::Model>(reg, m_currentEntity);
			ImGui::Component<comp::Script>(reg, m_currentEntity);
		
			
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

entt::entity EntityInspector::getEntity() const {
	return m_currentEntity;
}

void EntityInspector::setEntity(entt::entity id) {
	m_currentEntity = id;
}
