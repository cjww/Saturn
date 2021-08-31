#pragma once
//#include "ECS\Components.h"

#include "ECS/ECSCoordinator.h"
#include "ECS/Components.h"
#include "imgui.h"


#define IMGUI_BUFFER_SIZE_BIG 256U
#define IMGUI_BUFFER_SIZE_NORMAL 128U
#define IMGUI_BUFFER_SIZE_SMALL 64U
#define IMGUI_BUFFER_SIZE_TINY 32U

namespace ImGui {
	static struct PopupPayload {
		void* data;
		size_t size;
	} payload;

	void Component(Transform* transform);
	void Component(Model* model);
	void Component(Script* script);
	

	template<typename T>
	void Component(EntityID entity);



}

template<typename T>
void ImGui::Component(EntityID entity) {
	T* comp = ECSCoordinator::get()->getComponent<T>(entity);
	if (!comp) return;

	ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
	static bool visable = true;
	if (ImGui::CollapsingHeader(ECSCoordinator::get()->getComponentName<T>(), &visable)) {
		ImGui::Component(comp);
	}
	if (!visable) {
		ImGui::OpenPopup("Remove?");
		payload.data = (void*)ECSCoordinator::get()->getComponentType<T>();
		payload.size = sizeof(ComponentType);
		visable = true;
	}
}
