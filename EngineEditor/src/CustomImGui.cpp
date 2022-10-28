#include "pch.h"
#include "CustomImGui.h"

namespace ImGui {

	void displayLuaTable(std::string name, sol::table table) {
		bool open = ImGui::TreeNode(name.c_str());
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("table");
		}
		if (open) {	
			for (auto& [key, value] : table) {
				switch (value.get_type()) {
					case sol::type::userdata:
					{
						std::string valueAsStr = sa::LuaAccessable::getState()["tostring"](value);
						std::string str = key.as<std::string>() + " = " + valueAsStr;
						ImGui::Text(str.c_str());
						if(ImGui::IsItemHovered()) {
							ImGui::SetTooltip("userdata");
						}
						break;
					}
					case sol::type::table: 
					{
						std::string keyString;
						if (!key.is<int>()) {
							keyString = key.as<std::string>();
						}
						else {
							keyString = std::to_string(key.as<int>());
						}
						displayLuaTable(keyString, value.as<sol::table>());

						break;
					}
					case sol::type::function:
						ImGui::Text(key.as<std::string>().c_str());
						if (ImGui::IsItemHovered()) {
							ImGui::SetTooltip("function");
						}
						break;
					case sol::type::string: {
						std::string v = value.as<std::string>();
						ImGui::InputText(key.as<std::string>().c_str(), &v);
						table[key] = v;
						if (ImGui::IsItemHovered()) {
							ImGui::SetTooltip("string");
						}
						break;
					}
					case sol::type::number: {
						float v = value.as<float>();
						ImGui::InputFloat(key.as<std::string>().c_str(), &v);
						table[key] = v;
						if (ImGui::IsItemHovered()) {
							ImGui::SetTooltip("number");
						}
						break;
					}
					case sol::type::boolean: {
						bool v = value.as<bool>();
						ImGui::Checkbox(key.as<std::string>().c_str(), &v);
						table[key] = v;
						if (ImGui::IsItemHovered()) {
							ImGui::SetTooltip("boolean");
						}
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
		if (!transform->hasParent) {
			ImGui::DragFloat3("Position##Transform", (float*)&transform->position, 0.1f, 0.0f, 0.0f, "%.2f", ImGuiSliderFlags_NoRoundToFormat);
		}
		else {
			ImGui::DragFloat3("Position##Transform", (float*)&transform->relativePosition, 0.1f, 0.0f, 0.0f, "%.2f", ImGuiSliderFlags_NoRoundToFormat);
		}
		glm::vec3 rotation = glm::degrees(glm::eulerAngles(transform->rotation));
		if (ImGui::DragFloat3("Rotation", (float*)&rotation, 0.1f, 0.0f, 0.0f, "%.2f", ImGuiSliderFlags_NoRoundToFormat)) {
			transform->rotation = glm::quat(glm::radians(rotation));
		}
		ImGui::DragFloat3("Scale", (float*)&transform->scale, 0.1f, 0.0f, 0.0f, "%.2f", ImGuiSliderFlags_NoRoundToFormat);
	}

	void Component(comp::Model* model) {
		ImGui::Text("ModelID: %d", model->modelID);
		ImGui::SameLine();
		ImGui::Text("Name: %s", sa::ResourceManager::get().idToKey<sa::ModelData>(model->modelID).c_str());
		if (model->modelID != NULL_RESOURCE) {
			/*
			sa::ModelData* data = sa::AssetManager::get().getModel(model->modelID);
			for (const auto& mesh : data->meshes) {
				ImGui::Spacing();
				ImGui::Text("Material");
				ImGui::ColorEdit4("Diffuse Color", (float*)&mesh.material.diffuseColor);
			}
			*/

		}

	}

	void Component(comp::Script* script) {
		if (!script->env.valid())
			return;

		displayLuaTable("Environment", script->env);

	}

	void Component(comp::Light* light) {

		ImGui::DragFloat3("Position##Light", (float*)&light->values.position, 0.1f);

		ImGui::ColorEdit4("Color", (float*)&light->values.color);

		ImGui::SliderFloat("Intensity", &light->values.intensity, 0.1f, 1.f);
		ImGui::SliderFloat("Attenuation radius", &light->values.attenuationRadius, 2.f, 50.f);

		static std::string preview = "-";
		if (ImGui::BeginCombo("Type", preview.data())) {
			if (ImGui::Selectable("Point")) {
				light->values.type = sa::LightType::POINT;
				preview = "Point";
			}
			if (ImGui::Selectable("Directional")) {
				light->values.type = sa::LightType::DIRECTIONAL;
				preview = "Directional";
			}

			ImGui::EndCombo();
		}

	}

	void viewFile(std::filesystem::path filePath) {
		ImGui::Selectable(filePath.filename().string().c_str());
	}

	void viewDirectory(const std::filesystem::path& directory, std::filesystem::path& openDirectory) {
		//TODO: check permissions on directory before displaying
		/*
		auto status = std::filesystem::status(directory);
		std::filesystem::perms perm = status.permissions();
		std::filesystem::perms flags = std::filesystem::perms::owner_write | std::filesystem::perms::owner_read;
		*/
		
		bool opened = ImGui::TreeNodeEx(directory.filename().string().c_str(), ImGuiTreeNodeFlags_OpenOnArrow);
		if (ImGui::IsItemClicked()) {
			openDirectory = directory;
		}
		if (opened) {
			for (const auto& entry : std::filesystem::directory_iterator(directory)) {
				if (entry.is_directory()) {
					viewDirectory(entry.path(), openDirectory);
				}
				else {
					viewFile(entry.path());
				}
			}
			ImGui::TreePop();
		}
	}

	void DirectoryHierarchy(const char* str_id, const std::filesystem::path& directory, std::filesystem::path& openDirectory, int& iconSize, const ImVec2& size) {

		ImVec2 contentArea = size;
		if (size.x == 0.f && size.y == 0.f) {
			contentArea = ImGui::GetContentRegionAvail();
		}

		if (ImGui::BeginChild(str_id, contentArea)) {
			for (const auto& entry : std::filesystem::directory_iterator(directory)) {
				if (entry.is_directory()) {
					viewDirectory(entry.path(), openDirectory);
				}
				else {
					viewFile(entry.path());
				}
			}
			ImGui::EndChild();
		}

	}

	void DirectoryView(const char* str_id, const std::filesystem::path& directory, std::filesystem::path& openDirectory, int& iconSize, const ImVec2& size) {

		ImVec2 contentArea = size;
		if (size.x == 0.f && size.y == 0.f) {
			contentArea = ImGui::GetContentRegionAvail();
		}
		ImVec2 overviewArea = contentArea;
		overviewArea.x /= 4;
		
		ImVec2 viewArea = contentArea;
		viewArea.x -= overviewArea.x;

		DirectoryHierarchy(str_id, directory, openDirectory, iconSize, overviewArea);

		if (!openDirectory.empty()) {
			ImGui::SameLine();
			if (ImGui::BeginChild((std::string(str_id) + "1").c_str(), viewArea)) {
				
				ImGui::Text(openDirectory.string().c_str());
				ImGui::SameLine();

				ImGui::SetNextItemWidth(200);
				ImGui::SliderInt("Icon size", &iconSize, 20, 200);

				ImVec2 vecSize;
				vecSize.x = iconSize;
				vecSize.y = iconSize;
				
				int i = 0;
				for (const auto& entry : std::filesystem::directory_iterator(openDirectory)) {

					ImGui::Button("B", vecSize);
					ImGui::SameLine();
					
					if(ImGui::GetContentRegionMax().x - ImGui::GetCursorPosX() < vecSize.x) {
						ImGui::NewLine();
					}

				}
				ImGui::EndChild();
			}
		}

	}

	void DirectoryBrowser(const char* str_id, std::filesystem::path& directory, std::filesystem::path& openDirectory, int& iconSize, const ImVec2& size) {
		if (ImGui::ArrowButton((std::string("parentDir_") + str_id).c_str(), ImGuiDir_Left)) {
			directory = std::filesystem::absolute(directory);
			directory = directory.parent_path();
		}
		DirectoryView(str_id, directory, openDirectory, iconSize, size);
	}

	bool MakeEnterNameModalPopup(const char* name, const char* hint, std::string& output) {
		/*
		ImVec2 size = ImGui::GetContentRegionAvail();
		size.x /= 4;
		size.y /= 4;
		ImGui::SetNextWindowSize(size);
		*/

		ImGui::SetNextWindowContentSize(ImVec2(300, 100));

		if (ImGui::BeginPopupModal(name, 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings)) {

			ImGui::Text("Enter name: ");
			static bool setFocus = true;
			if (setFocus) {
				ImGui::SetKeyboardFocusHere();
				setFocus = false;
			}
			bool pressedEnter = ImGui::InputTextWithHint("Name", hint, &output, ImGuiInputTextFlags_EnterReturnsTrue);
			static std::string erromsg;
			ImGui::Spacing();
			bool pressedOK = false;
			if (ImGui::Button("Ok") || pressedEnter) {
				if (!output.empty()) {
					ImGui::CloseCurrentPopup();
					erromsg = "";
					setFocus = true;
					pressedOK = true;
				}
				else {
					erromsg = "Please enter a name";
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) {
				ImGui::CloseCurrentPopup();
				erromsg = "";
				setFocus = true;
			}
			if (!erromsg.empty()) {
				ImGui::TextColored(IMGUI_COLOR_ERROR_RED, erromsg.c_str());
			}
			ImGui::EndPopup();
			return pressedOK;
		}
		return false;
	}

	bool ProjectButton(const char* name, const char* path) {
		ImVec2 size(GetWindowContentRegionWidth(), 50);
		
		ImVec2 sizeMin, sizeMax;
		sizeMin = ImVec2(GetCursorPos().x + GetWindowPos().x, GetCursorPos().y + GetWindowPos().y);
		sizeMax = ImVec2(sizeMin.x + size.x, sizeMin.y + size.y);

		bool isHovered = IsMouseHoveringRect(sizeMin, sizeMax);
		
		PushStyleVar(ImGuiStyleVar_ChildRounding, ImGui::GetStyle().FrameRounding);
		PushStyleVar(ImGuiStyleVar_ChildBorderSize, ImGui::GetStyle().FrameBorderSize);
		PushStyleVar(ImGuiStyleVar_WindowPadding, ImGui::GetStyle().FramePadding);

		if (isHovered && IsMouseDown(ImGuiMouseButton_Left)) {
			PushStyleColor(ImGuiCol_ChildBg, GetColorU32(ImGuiCol_ButtonActive));
		}
		else if (isHovered) {
			PushStyleColor(ImGuiCol_ChildBg, GetColorU32(ImGuiCol_ButtonHovered));
		}
		else {
			PushStyleColor(ImGuiCol_ChildBg, GetColorU32(ImGuiCol_Button));
		}

		BeginChild(path, ImVec2(GetWindowContentRegionWidth(), 50));
		
		SetCursorPosX(GetCursorPosX() + GetStyle().FramePadding.x);
		SetCursorPosY(GetCursorPosY() + GetStyle().FramePadding.y);

		Text(name);

		SetCursorPosX(GetCursorPosX() + GetStyle().FramePadding.x);
		SetCursorPosY(GetCursorPosY() + GetStyle().FramePadding.y);

		BeginDisabled();
		Text(path);
		EndDisabled();

		EndChild();
		
		PopStyleVar(3);
		PopStyleColor();
		

		return IsItemClicked();
	}

	
}
