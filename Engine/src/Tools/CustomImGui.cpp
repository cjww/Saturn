#include "pch.h"
#include "CustomImGui.h"

namespace ImGui {

	void displayTable(std::string name, sol::table table) {
		if (ImGui::TreeNode(name.c_str())) {
			for (auto& [key, value] : table) {
				switch (value.get_type()) {
					case sol::type::userdata:
						ImGui::Text("%s : userdata", key.as<std::string>().c_str());
						break;
					case sol::type::table:
						displayTable(key.as<std::string>(), value.as<sol::table>());
						break;
					case sol::type::function:
						ImGui::Text("%s : function", key.as<std::string>().c_str());
						break;
			
					case sol::type::string: {
						std::string v = value.as<std::string>();
						ImGui::InputText(key.as<std::string>().c_str(), &v);
						table[key] = v;
						break;
					}
					case sol::type::number: {
						float v = value.as<float>();
						ImGui::InputFloat(key.as<std::string>().c_str(), &v);
						table[key] = v;
						break;
					}
					case sol::type::boolean: {
						bool v = value.as<bool>();
						ImGui::Checkbox(key.as<std::string>().c_str(), &v);
						table[key] = v;
						break;
					}
					default:
						ImGui::Text("nil");
						break;
				}

			}

			ImGui::TreePop();
		}
	}

	void Component(comp::Transform* transform) {
		ImGui::DragFloat3("Position", (float*)&transform->position, 0.1f);
		ImGui::DragFloat3("Rotation", (float*)&transform->rotation, 0.1f);
		ImGui::DragFloat3("Scale", (float*)&transform->scale, 0.1f);
	}

	void Component(comp::Model* model) {
		ImGui::Text("ModelID: %d", model->modelID);
		ImGui::SameLine();
		ImGui::Text("Name: %s", sa::ResourceManager::get().idToKey(model->modelID).c_str());
		if (model->modelID != NULL_RESOURCE) {
			sa::ModelData* data = sa::AssetManager::get().getModel(model->modelID);
			for (const auto& mesh : data->meshes) {
				ImGui::Spacing();
				ImGui::Text("Material");
				ImGui::ColorEdit4("Diffuse Color", (float*)&mesh.material.diffuseColor);
			}
		}
		
	}

	void Component(comp::Script* script) {
		if (!script->env.valid())
			return;

		displayTable("Environment", script->env);

	}

	
}
