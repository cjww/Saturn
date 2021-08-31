#include "CustomImGui.h"


namespace ImGui {

	void Component(Transform* transform) {
		ImGui::DragFloat3("Position", (float*)&transform->position, 0.1f);
		ImGui::DragFloat3("Rotation", (float*)&transform->rotation, 0.1f);
		ImGui::DragFloat3("Scale", (float*)&transform->scale, 0.1f);
	}

	void Component(Model* model) {
		ImGui::Text("ModelID: %d", model->modelID);
		sa::ModelData* data = sa::ResourceManager::get()->getModel(model->modelID);
		for (const auto& mesh : data->meshes) {
			ImGui::Spacing();
			ImGui::Text("Material");
			ImGui::ColorEdit4("Diffuse Color", (float*)&mesh.material.diffuseColor);
		}
		
	}
	void Component(Script* script) {
		char buffer[IMGUI_BUFFER_SIZE_NORMAL];
		strcpy_s(buffer, script->scriptName.c_str());
		ImGui::InputText("Name##Script", buffer, IMGUI_BUFFER_SIZE_NORMAL);
	}
	
}
