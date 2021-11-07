#pragma once
//#include "ECS\Components.h"

#include <entt/entt.hpp>

#include "ECS/ECSCoordinator.h"
#include "ECS/Components.h"
#include "imgui.h"


#define IMGUI_BUFFER_SIZE_BIG 256U
#define IMGUI_BUFFER_SIZE_NORMAL 128U
#define IMGUI_BUFFER_SIZE_SMALL 64U
#define IMGUI_BUFFER_SIZE_TINY 32U

namespace ImGui {
	static struct PopupPayload {
		entt::type_info type;
	} payload;

	void Component(comp::Transform* transform);
	void Component(comp::Model* model);
	void Component(comp::Script* script);
	

	template<typename T>
	void Component(entt::registry& reg, entt::entity entity);



}

template<typename T>
void ImGui::Component(entt::registry& reg, entt::entity entity) {
	T* comp = reg.try_get<T>(entity);
	if (!comp) return;


	ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
	static bool visable = true;
	if (ImGui::CollapsingHeader(ECSCoordinator::get()->getComponentName<T>(), &visable)) {
		ImGui::Component(comp);
	}
	if (!visable) {
		ImGui::OpenPopup("Remove?");
		//payload.data = (void*)ECSCoordinator::get()->getComponentType<T>();
		payload.type = entt::type_id<T>();
		visable = true;
	}
}
