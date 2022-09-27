#pragma once

#include "ECS/Components.h"
#include "ECS/Entity.h"

#define IMGUI_BUFFER_SIZE_BIG 256U
#define IMGUI_BUFFER_SIZE_NORMAL 128U
#define IMGUI_BUFFER_SIZE_SMALL 64U
#define IMGUI_BUFFER_SIZE_TINY 32U

namespace ImGui {
	static struct PopupPayload {
		sa::ComponentType type;
		std::string name;
	} payload;

	void displayLuaTable(std::string name, sol::table table);

	void Component(comp::Transform* transform);
	void Component(comp::Model* model);
	void Component(comp::Script* script);
	void Component(comp::Light* light);


	template<typename T>
	void Component(const sa::Entity& entity);



}

template<typename T>
void ImGui::Component(const sa::Entity& entity) {
	T* comp = entity.getComponent<T>();
	if (!comp) return;


	ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
	static bool visable = true;
	
	if (ImGui::CollapsingHeader(sa::getComponentName<T>().c_str(), &visable)) {
		ImGui::Component(comp);
	}
	if (!visable) {
		ImGui::OpenPopup("Remove?");
		payload.type = sa::getComponentType<T>();
		visable = true;
	}
}
