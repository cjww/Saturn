#include "LuaConsole.h"

#include "ScriptManager.h" // to get lua state

void LuaConsole::update(float dt) {

}

void LuaConsole::onImGui() {
	//ImGui::ShowDemoWindow();
	//ImGui::ShowStyleEditor();

	sol::state& lua = sa::LuaAccessable::getState();
	if (ImGui::Begin("Lua Console")) {
		if (ImGui::InputText("Input", m_inputBuffer, ImGuiInputTextFlags_EnterReturnsTrue)) {
			auto result = lua.do_string(m_inputBuffer);
			if (!result.valid()) {
				SA_DEBUG_LOG_ERROR(lua_tostring(lua, -1));
			}
		}
	}
	ImGui::End();

}