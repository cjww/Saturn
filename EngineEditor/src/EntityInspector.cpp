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

	if (ImGui::BeginPopup("Remove script?")) {
		ImGui::Text("Remove?");
		if (ImGui::Button("Yes")) {
			ImGui::CloseCurrentPopup();
			m_pEngine->getCurrentScene()->removeScript(m_selectedEntity, ImGui::payload.name);
		}
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("Select Component")) {
		for (auto type : sa::ComponentType::getRegisteredComponents()) {
			if (type == sa::getComponentType<comp::Name>())
				continue;
			if (ImGui::Button(type.getName().c_str())) {
				m_selectedEntity.addComponent(type);
				ImGui::CloseCurrentPopup();
			}
		}
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("Select Script")) {
		std::string buffer;
		if(ImGui::InputText("Script Name", &buffer, ImGuiInputTextFlags_EnterReturnsTrue)) {
			m_pEngine->getCurrentScene()->addScript(m_selectedEntity, buffer);
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

}

EntityInspector::EntityInspector(sa::Engine* pEngine) : EditorModule(pEngine) {
	m_selectedEntity = {};

	pEngine->on<sa::engine_event::SceneSet>([&](const sa::engine_event::SceneSet& sceneSetEvent, sa::Engine& engine) {
		sceneSetEvent.newScene->on<sa::editor_event::EntitySelected>([&](const sa::editor_event::EntitySelected& e, sa::Scene&) {
			m_selectedEntity = e.entity;
		});

		sceneSetEvent.newScene->on<sa::editor_event::EntityDeselected>([&](const sa::editor_event::EntityDeselected&, sa::Scene&) {
			m_selectedEntity = {};
		});
	});
}

EntityInspector::~EntityInspector() {

}

void EntityInspector::onImGui() {
	SA_PROFILE_FUNCTION();

	if (ImGui::Begin("Inspector")) {

		if (m_selectedEntity) {
			
			makePopups();
			
			char buffer[IMGUI_BUFFER_SIZE_TINY];
			comp::Name* nameComp = m_selectedEntity.getComponent<comp::Name>();

			
			strcpy_s(buffer, nameComp->name.c_str());
			if (ImGui::InputText("Name##Entity", buffer, IMGUI_BUFFER_SIZE_TINY, ImGuiInputTextFlags_EnterReturnsTrue)) {
				nameComp->name = buffer;
			}
			
			ImGui::Component<comp::Transform>(m_selectedEntity);
			ImGui::Component<comp::Model>(m_selectedEntity);
			ImGui::Component<comp::Script>(m_selectedEntity);
			ImGui::Component<comp::Light>(m_selectedEntity);

			// Display entity scripts
			for (auto& script : m_pEngine->getCurrentScene()->getAssignedScripts(m_selectedEntity)) {
				ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
				static bool visable = true;

				if (ImGui::CollapsingHeader(script.name.c_str(), &visable)) {
					ImGui::displayLuaTable("Environment##" + script.name, script.env);
				}
				if (!visable) {
					ImGui::OpenPopup("Remove script?");
					ImGui::payload.name = script.name;
					visable = true;
				}
			}


			
			ImGui::Separator();
			ImGui::Spacing();
			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() / 3);
			if (ImGui::Button("Add Component +", ImVec2(-ImGui::GetWindowContentRegionWidth() / 3.f, 0))) {
				ImGui::OpenPopup("Select Component");
			}
			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() / 3);
			if (ImGui::Button("Add Script +", ImVec2(-ImGui::GetWindowContentRegionWidth() / 3.f, 0))) {
				ImGui::OpenPopup("Select Script");
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
