#include "LuaConsole.h"

#include "ScriptManager.h" // to get lua state


void LuaConsole::update(float dt) {

}

void LuaConsole::onImGui() {
	//ImGui::ShowDemoWindow();
	//ImGui::ShowStyleEditor();

	if (ImGui::Begin("Lua Console")) {
		ImGui::BeginChild("Scroll_Area");

		//ImGui::TextUnformatted();
		for (size_t i = 0; i < m_items.size(); i++) {
			LogItem& item = m_items[i];
			bool popColor = true;
			switch (item.type) {
			case LogType::INFO:
				popColor = false;
				break;
			case LogType::ERROR:
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
				break;
			case LogType::SUCCESS:
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 1.0f, 0.4f, 1.0f));
				break;
			case LogType::WARNING:
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 1.0f, 1.0f, 1.0f));
				break;
			default:
				popColor = false;
				break;
			}

			ImGui::TextUnformatted(item.msg.c_str());

			if (popColor)
				ImGui::PopStyleColor();

		}

		ImGui::EndChild();

		sol::state& lua = sa::LuaAccessable::getState();
		if (ImGui::InputText("Input", m_inputBuffer, 256, ImGuiInputTextFlags_EnterReturnsTrue)) {
			auto result = lua.do_string(m_inputBuffer);
			if (!result.valid()) {
				m_items.emplace_back(lua_tostring(lua, -1), LogType::ERROR);
			}
		}
	}
	ImGui::End();

}