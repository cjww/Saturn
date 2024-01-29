#include "EntityInspector.h"
#include "EngineEditor.h"

void EntityInspector::makePopups() {
	if (ImGui::BeginPopup("Remove?")) {
		ImGui::Text("Remove %s?", ImGui::payload.type.getName().c_str());
		if (ImGui::Button("Yes")) {
			ImGui::CloseCurrentPopup();
			m_selectedEntity.removeComponent(ImGui::payload.type);
		}
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("Remove script?")) {
		ImGui::Text("Remove %s?", ImGui::payload.name.c_str());
		if (ImGui::Button("Yes")) {
			ImGui::CloseCurrentPopup();
			m_selectedEntity.removeScript(ImGui::payload.name);
		}
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("Select Component")) {
		for (auto type : sa::ComponentType::getRegisteredComponents()) {
			if (type == sa::getComponentType<comp::Name>())
				continue;
			if (ImGui::MenuItem(type.getName().c_str())) {
				m_selectedEntity.addComponent(type);
			}
		}
		ImGui::EndPopup();
	}
	static bool fetchedScripts = false;
	static std::string filter;
	if (ImGui::BeginPopup("Select Script")) {
		static auto paths = m_pEditor->fetchAllScriptsInProject();
		if (!fetchedScripts) {
			paths = m_pEditor->fetchAllScriptsInProject();
			fetchedScripts = true;
		}

		if (ImGui::BeginListBox("##ScriptFileList")) {

			for (auto& path : paths) {
				std::string scriptName = sa::utils::toLower(path.filename().generic_string());
				if (scriptName.find(sa::utils::toLower(filter)) == std::string::npos) {
					continue;
				}
				if (ImGui::Selectable(scriptName.c_str())) {
					m_selectedEntity.addScript(path);
					ImGui::CloseCurrentPopup();
				}

			}

			ImGui::EndListBox();
		}
		
		ImGui::InputText("##ScriptFilter", &filter);
		int height = ImGui::GetItemRectSize().y;
		
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		ImGui::SameLine();
		ImGui::PopStyleVar();

		if (ImGui::Button("X", ImVec2(height, height))) {
			filter = "";
		}
		
		ImGui::EndPopup();
	}
	else {
		fetchedScripts = false;
		filter = "";
	}

}

void EntityInspector::onEntitySelected(const sa::editor_event::EntitySelected& e) {
	m_selectedEntity = e.entity;
}

void EntityInspector::onEntityDeselected(const sa::editor_event::EntityDeselected& e) {
	m_selectedEntity = {};
}

EntityInspector::EntityInspector(sa::Engine* pEngine, sa::EngineEditor* pEditor) 
	: EditorModule(pEngine, pEditor, "Inspector", false) {
	m_selectedEntity = {};

	pEngine->sink<sa::editor_event::EntitySelected>().connect<&EntityInspector::onEntitySelected>(this);
	pEngine->sink<sa::editor_event::EntityDeselected>().connect<&EntityInspector::onEntityDeselected>(this);
}

EntityInspector::~EntityInspector() {

}

void EntityInspector::onImGui() {
	SA_PROFILE_FUNCTION();
	
	if (ImGui::Begin(m_name) && m_pEngine->getCurrentScene()) {

		if (m_selectedEntity) {
			
			makePopups();
			
			char buffer[IMGUI_BUFFER_SIZE_TINY];
			comp::Name* nameComp = m_selectedEntity.getComponent<comp::Name>();

			
			strcpy_s(buffer, nameComp->name.c_str());
			if (ImGui::InputText("Name##Entity", buffer, IMGUI_BUFFER_SIZE_TINY, ImGuiInputTextFlags_EnterReturnsTrue)) {
				nameComp->name = buffer;
			}
			ImGui::SameLine();
			ImGui::Text("ID: %u", (uint32_t)m_selectedEntity);
			
			ImGui::Component<comp::Transform>(m_selectedEntity);
			ImGui::Component<comp::Model>(m_selectedEntity);
			ImGui::Component<comp::Light>(m_selectedEntity);
			ImGui::Component<comp::RigidBody>(m_selectedEntity);
			ImGui::Component<comp::BoxCollider>(m_selectedEntity);
			ImGui::Component<comp::SphereCollider>(m_selectedEntity);
			ImGui::Component<comp::Camera>(m_selectedEntity);


			// Display entity scripts
			auto scripts = m_pEngine->getCurrentScene()->getAssignedScripts(m_selectedEntity);
			for (auto& pScript : scripts) {
				ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
				static bool visable = true;

				if(ImGui::Script(pScript, &visable)) {
					m_pEngine->getCurrentScene()->reloadScript(pScript);
				}

				if (!visable) {
					ImGui::OpenPopup("Remove script?");
					ImGui::payload.name = pScript->name;
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
