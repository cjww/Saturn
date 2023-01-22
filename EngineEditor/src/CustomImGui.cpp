#include "CustomImGui.h"
#include "Tools\Math.h"

namespace ImGui {

	void SetupImGuiStyle() {
		// Fork of Photoshop style from ImThemes
		ImGuiStyle& style = ImGui::GetStyle();

		style.Alpha = 1.0f;
		style.DisabledAlpha = 0.6000000238418579f;
		style.WindowPadding = ImVec2(8.0f, 8.0f);
		style.WindowRounding = 4.0f;
		style.WindowBorderSize = 1.0f;
		style.WindowMinSize = ImVec2(32.0f, 32.0f);
		style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
		style.WindowMenuButtonPosition = ImGuiDir_Right;
		style.ChildRounding = 3.200000047683716f;
		style.ChildBorderSize = 1.0f;
		style.PopupRounding = 2.0f;
		style.PopupBorderSize = 1.0f;
		style.FramePadding = ImVec2(4.0f, 3.0f);
		style.FrameRounding = 2.0f;
		style.FrameBorderSize = 1.0f;
		style.ItemSpacing = ImVec2(8.0f, 4.0f);
		style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
		style.CellPadding = ImVec2(4.0f, 2.0f);
		style.IndentSpacing = 21.0f;
		style.ColumnsMinSpacing = 6.0f;
		style.ScrollbarSize = 10.0f;
		style.ScrollbarRounding = 11.19999980926514f;
		style.GrabMinSize = 6.099999904632568f;
		style.GrabRounding = 0.8999999761581421f;
		style.TabRounding = 0.0f;
		style.TabBorderSize = 1.0f;
		style.TabMinWidthForCloseButton = 0.0f;
		style.ColorButtonPosition = ImGuiDir_Right;
		style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
		style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

		style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.3519313335418701f, 0.3519278168678284f, 0.3519278168678284f, 1.0f);
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1802574992179871f, 0.1802556961774826f, 0.1802556961774826f, 1.0f);
		style.Colors[ImGuiCol_ChildBg] = ImVec4(0.2784313857555389f, 0.2784313857555389f, 0.2784313857555389f, 0.0f);
		style.Colors[ImGuiCol_PopupBg] = ImVec4(0.1803921610116959f, 0.1803921610116959f, 0.1803921610116959f, 1.0f);
		style.Colors[ImGuiCol_Border] = ImVec4(0.2017167210578918f, 0.2008509933948517f, 0.2008509933948517f, 1.0f);
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.05150216817855835f, 0.05150165408849716f, 0.05150165408849716f, 0.3991416096687317f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.1568627506494522f, 0.1568627506494522f, 0.1568627506494522f, 1.0f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2000000029802322f, 1.0f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.2784313857555389f, 0.2784313857555389f, 0.2784313857555389f, 1.0f);
		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1450980454683304f, 1.0f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1450980454683304f, 1.0f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1450980454683304f, 1.0f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.1921568661928177f, 0.1921568661928177f, 0.1921568661928177f, 1.0f);
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.1568627506494522f, 0.1568627506494522f, 0.1568627506494522f, 1.0f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.2745098173618317f, 0.2745098173618317f, 0.2745098173618317f, 1.0f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.2980392277240753f, 0.2980392277240753f, 0.2980392277240753f, 1.0f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.3882353007793427f, 0.3882353007793427f, 0.3882353007793427f, 1.0f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
		style.Colors[ImGuiCol_Button] = ImVec4(0.3690987229347229f, 0.3690950274467468f, 0.3690950274467468f, 0.0f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.1560000032186508f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.3004291653633118f, 0.3004277646541595f, 0.3004261553287506f, 0.5622317790985107f);
		style.Colors[ImGuiCol_Header] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3098039329051971f, 1.0f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.4666666686534882f, 0.4666666686534882f, 0.4666666686534882f, 1.0f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.4666666686534882f, 0.4666666686534882f, 0.4666666686534882f, 1.0f);
		style.Colors[ImGuiCol_Separator] = ImVec4(0.2627451121807098f, 0.2627451121807098f, 0.2627451121807098f, 1.0f);
		style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.3882353007793427f, 0.3882353007793427f, 0.3882353007793427f, 1.0f);
		style.Colors[ImGuiCol_SeparatorActive] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
		style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.0f, 1.0f, 1.0f, 0.25f);
		style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.6700000166893005f);
		style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
		style.Colors[ImGuiCol_Tab] = ImVec4(0.1416308879852295f, 0.1416294723749161f, 0.1416294723749161f, 1.0f);
		style.Colors[ImGuiCol_TabHovered] = ImVec4(0.3490196168422699f, 0.3490196168422699f, 0.3490196168422699f, 1.0f);
		style.Colors[ImGuiCol_TabActive] = ImVec4(0.1803921610116959f, 0.1803921610116959f, 0.1803921610116959f, 1.0f);
		style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.09411764889955521f, 0.09411764889955521f, 0.09411764889955521f, 1.0f);
		style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.1921568661928177f, 0.1921568661928177f, 0.1921568661928177f, 1.0f);
		style.Colors[ImGuiCol_PlotLines] = ImVec4(0.4666666686534882f, 0.4666666686534882f, 0.4666666686534882f, 1.0f);
		style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.5843137502670288f, 0.5843137502670288f, 0.5843137502670288f, 1.0f);
		style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
		style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.2000000029802322f, 1.0f);
		style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3490196168422699f, 1.0f);
		style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.2274509817361832f, 0.2274509817361832f, 0.2470588237047195f, 1.0f);
		style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.05999999865889549f);
		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(1.0f, 1.0f, 1.0f, 0.1560000032186508f);
		style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
		style.Colors[ImGuiCol_NavHighlight] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
		style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 0.3882353007793427f, 0.0f, 1.0f);
		style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.5860000252723694f);
		style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.5860000252723694f);
	}

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
						if (value.is<sa::Vector2>()) {
							sa::Vector2& vec2 = value.as<sa::Vector2>();
							if (ImGui::DragFloat2(key.as<std::string>().c_str(), (float*)&vec2, 0.5f)) {
								table[key] = vec2;
							}
							if (ImGui::IsItemHovered()) {
								ImGui::SetTooltip("Vec2");
							}
						}
						else if (value.is<sa::Vector3>()) {
							sa::Vector3& vec3 = value.as<sa::Vector3>();
							if (ImGui::DragFloat3(key.as<std::string>().c_str(), (float*)&vec3, 0.5f)) {
								table[key] = vec3;
							}
							if (ImGui::IsItemHovered()) {
								ImGui::SetTooltip("Vec3");
							}
						}
						else if (value.is<sa::Vector4>()) {
							sa::Vector4& vec4 = value.as<sa::Vector4>();
							if (ImGui::DragFloat4(key.as<std::string>().c_str(), (float*)&vec4, 0.5f)) {
								table[key] = vec4;
							}
							ImGui::SameLine();
							
							ImVec4 col = { vec4.x, vec4.y, vec4.z, vec4.w };
							if (ImGui::ColorButton("as_color", col)) {
								ImGui::OpenPopup("##ColorPicker");
							}
							
							if (ImGui::BeginPopup("##ColorPicker")) {
								ImGui::ColorPicker3("##Vec4AsColor", (float*)&vec4);
								ImGui::EndPopup();
							}
							
							if (ImGui::IsItemHovered()) {
								ImGui::SetTooltip("Vec4");
							}
						}
						else {
							std::string valueAsStr = sa::LuaAccessable::getState()["tostring"](value);
							std::string str = key.as<std::string>() + " = " + valueAsStr;
							ImGui::Text(str.c_str());
							if(ImGui::IsItemHovered()) {
								ImGui::SetTooltip("userdata");
							}
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
						if (ImGui::InputText(key.as<std::string>().c_str(), &v, ImGuiInputTextFlags_EnterReturnsTrue)) {
							table[key] = v;
						}
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

	void Component(sa::Entity entity, comp::Transform* transform) {
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

	void Component(sa::Entity entity, comp::Model* model) {
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

	void Component(sa::Entity entity, comp::Script* script) {
		if (!script->env.valid())
			return;

		displayLuaTable("Environment", script->env);

	}

	void Component(sa::Entity entity, comp::Light* light) {

		ImGui::ColorEdit4("Color", (float*)&light->values.color);

		ImGui::SliderFloat("Intensity", &light->values.intensity, 0.0f, 10.f);
		ImGui::SliderFloat("Attenuation radius", &light->values.attenuationRadius, 0.0, 200.f);

		static std::string preview = "Point";
		if (ImGui::BeginCombo("Type", preview.c_str())) {
			if (ImGui::Selectable("Point", light->values.type == sa::LightType::POINT)) {
				light->values.type = sa::LightType::POINT;
				preview = "Point";
			}
			if (ImGui::Selectable("Directional", light->values.type == sa::LightType::DIRECTIONAL)) {
				light->values.type = sa::LightType::DIRECTIONAL;
				preview = "Directional";
			}

			ImGui::EndCombo();
		}

	}

	void Component(sa::Entity entity, comp::RigidBody* rb) {
		if (ImGui::Checkbox("Static", &rb->isStatic)) {
			entity.update<comp::RigidBody>();
		}


	}

	void Component(sa::Entity entity, comp::BoxCollider* bc) {
		if (ImGui::DragFloat3("Half Length##BoxCollider", (float*)&bc->halfLengths, 0.1f, 0.01f)) {
			bc->onUpdate(&entity);
		}
		if (ImGui::DragFloat3("Offset##BoxCollider", (float*)&bc->offset)) {
			bc->onUpdate(&entity);
		}

	}

	void Component(sa::Entity entity, comp::SphereCollider* sc) {
		if (ImGui::DragFloat("Radius##SphereCollider", &sc->radius, 0.1f, 0.1f)) {
			sc->onUpdate(&entity);
		}
		if (ImGui::DragFloat3("Offset##SphereCollider", (float*)&sc->offset)) {
			sc->onUpdate(&entity);
		}
	}

	void Component(sa::Entity entity, comp::Camera* camera) {
		ImGui::Checkbox("Is Primary", &camera->isPrimary);

		glm::vec3 position = camera->camera.getPosition();
		if (ImGui::DragFloat3("Position##Camera", (float*)&position)) {
			camera->camera.setPosition(position);
		}
		sa::Rect rect = camera->camera.getViewport();
		ImGui::Text("Viewport");
		ImGui::Indent();
		if (ImGui::DragInt2("Offset##Camera", (int32_t*)&rect.offset)) {
			camera->camera.setViewport(rect);
		}

		glm::ivec2 extent(rect.extent.width, rect.extent.height);
		if (ImGui::DragInt2("Extent##Camera", (int32_t*)&extent)) {
			rect.extent = { (uint32_t)extent.x, (uint32_t)extent.y };
			camera->camera.setViewport(rect);
		}
		ImGui::Unindent();




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

	bool ImageButtonTinted(const sa::Texture& tex, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1) {
		ImColor tintColor(1.f, 1.f, 1.f);
		ImColor hoveredColor = ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);
		ImColor activeColor = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0);

		ImVec2 rectMin = ImGui::GetCursorPos();
		ImVec2 windowPos = ImGui::GetWindowPos();
		rectMin.x += windowPos.x;
		rectMin.y += windowPos.y;
		ImVec2 rectMax(rectMin.x + size.x, rectMin.y + size.y);
		
		bool pressed = false;

		if (ImGui::IsMouseHoveringRect(rectMin, rectMax)) {
			tintColor = hoveredColor;
			if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
				pressed = true;
			}
			else if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
				tintColor = activeColor;
			}
		}
		
		ImGui::ImageButton(tex, size, uv0, uv1, 0, ImVec4(0, 0, 0, 0), tintColor);

		ImGui::PopStyleVar();
		ImGui::PopStyleColor(3);
		return pressed;
	}

	void GizmoIcon(const sa::Texture* pTex, const glm::vec3& worldPoint, const sa::SceneCamera* pCamera, const glm::vec2& rectPos, const glm::vec2& rectSize, int iconSize, ImColor tintColor) {
		ImVec2 windowPos = ImGui::GetWindowPos();
		glm::vec3 point = sa::math::worldToScreen(worldPoint, pCamera, rectPos, rectSize);
		if (point.z < 1) {
			
			iconSize /= glm::distance(worldPoint, pCamera->getPosition());
			ImGui::GetWindowDrawList()->AddImageQuad(
				sa::Renderer::get().getImGuiTexture(pTex),
				ImVec2(point.x - iconSize, point.y - iconSize),
				ImVec2(point.x + iconSize, point.y - iconSize),
				ImVec2(point.x + iconSize, point.y + iconSize),
				ImVec2(point.x - iconSize, point.y + iconSize),
				ImVec2(0, 0), ImVec2(1, 0),
				ImVec2(1, 1), ImVec2(0, 1),
				tintColor);
		}
	}

	void GizmoCircle(const glm::vec3& worldPosition, float radius, const glm::quat& rotation, const sa::SceneCamera* pCamera, const glm::vec2& screenPos, const glm::vec2& screenSize, const ImColor& color, int numSegments, float thickness) {
		if (numSegments == 0) 
			numSegments = 64;
		else if (numSegments > 128) 
			numSegments = 128;

		constexpr float twoPi = glm::radians(360.f);
		ImVec2 points[128];
		int pointCount = 0;
		float step = twoPi / numSegments;
		for (float angle = 0.f; angle < twoPi; angle += step) {
			glm::vec3 worldPoint(cos(angle), sin(angle), 0.f);
			worldPoint *= radius;
			worldPoint = glm::rotate(rotation, worldPoint);
			worldPoint += worldPosition;

			glm::vec3 screenPoint = sa::math::worldToScreen(worldPoint, pCamera, screenPos, screenSize);
			if (screenPoint.z > 1.f)
				continue;

			points[pointCount] = { screenPoint.x, screenPoint.y };
			pointCount++;
		}
		ImGui::GetWindowDrawList()->AddPolyline(points, pointCount, color, ImDrawFlags_Closed, thickness);

	}

	bool GizmoCircleResizable(const glm::vec3& worldPosition, float& radius, const glm::quat& rotation, const sa::SceneCamera* pCamera, const glm::vec2& screenPos, const glm::vec2& screenSize, const ImColor& color, bool& isDragging, int numSegments, float thickness) {
		if (numSegments == 0)
			numSegments = 64;
		else if (numSegments > 128)
			numSegments = 128;

		constexpr float twoPi = glm::radians(360.f);
		ImVec2 points[128];
		int pointCount = 0;
		float step = twoPi / numSegments;
		for (float angle = 0.f; angle < twoPi; angle += step) {
			glm::vec3 worldPoint(cos(angle), sin(angle), 0.f);
			worldPoint *= radius;
			worldPoint = glm::rotate(rotation, worldPoint);
			worldPoint += worldPosition;

			glm::vec3 screenPoint3D = sa::math::worldToScreen(worldPoint, pCamera, screenPos, screenSize);
			
			if (screenPoint3D.z > 1.f)
				continue;

			glm::vec2 screenPoint = screenPoint3D;
			if (pointCount == 0) {
				ImVec2 rectMin(screenPoint.x - HandleSize, screenPoint.y - HandleSize);
				ImVec2 rectMax(screenPoint.x + HandleSize, screenPoint.y + HandleSize);

				ImGui::GetWindowDrawList()->AddCircleFilled({ screenPoint.x, screenPoint.y }, HandleSize, color);

				ImVec2 windowPos = ImGui::GetWindowPos();
				ImGui::SetCursorPos(ImVec2(rectMin.x - windowPos.x, rectMin.y - windowPos.y));
				ImGui::InvisibleButton("circle_handle", ImVec2(rectMax.x - rectMin.x, rectMax.y - rectMin.y));
				bool isOver = ImGui::IsItemHovered();
				if (isOver) 
					ImGui::GetWindowDrawList()->AddCircleFilled({ screenPoint.x, screenPoint.y }, HandleSize, ImColor(1.f, 1.f, 1.f));

				if (isOver && ImGui::IsMouseDown(ImGuiMouseButton_Left))
					isDragging = true;
				else if (isDragging && !ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
					isDragging = false;
					return true;
				}

				if (isDragging) {
					glm::vec2 center = sa::math::worldToScreen(worldPosition, pCamera, screenPos, screenSize);

					float screenRadius = glm::distance(center, screenPoint);
					float ratio = radius / screenRadius;

					ImVec2 imDelta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left, 0.0f);
					ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
					glm::vec2 delta = { imDelta.x, imDelta.y };
					screenPoint += delta;

					screenRadius = glm::distance(center, screenPoint);
					radius = screenRadius * ratio;


					ImGui::GetWindowDrawList()->AddCircleFilled({ screenPoint.x, screenPoint.y }, HandleSize * 1.5f, color);
				}
			}
			points[pointCount] = { screenPoint.x, screenPoint.y };
			pointCount++;
		}
		ImGui::GetWindowDrawList()->AddPolyline(points, pointCount, color, ImDrawFlags_Closed, thickness);
		
		return false;
	}

	bool GizmoSphereResizable(const glm::vec3& worldPosition, float& radius, const glm::quat& rotation, const sa::SceneCamera* pCamera, const glm::vec2& screenPos, const glm::vec2& screenSize, const ImColor& color, bool disabled, int numSegments, float thickness) {
		static bool dragFirstCircle = false;
		static bool dragSecondCircle = false;

		if (disabled) {
			ImGui::GizmoCircle(worldPosition, radius, rotation, pCamera, screenPos, screenSize, color, numSegments, thickness);
			ImGui::GizmoCircle(worldPosition, radius, rotation * glm::quat(glm::vec3(glm::radians(90.f), 0, 0)), pCamera, screenPos, screenSize, color, numSegments, thickness);
			ImGui::GizmoCircle(worldPosition, radius, rotation * glm::quat(glm::vec3(0, glm::radians(90.f), 0)), pCamera, screenPos, screenSize, color, numSegments, thickness);
			return false;
		}

		bool released = ImGui::GizmoCircleResizable(worldPosition, radius, rotation, pCamera, screenPos, screenSize, color, dragFirstCircle, numSegments, thickness);
		ImGui::GizmoCircle(worldPosition, radius, rotation * glm::quat(glm::vec3(glm::radians(90.f), 0, 0)), pCamera, screenPos, screenSize, color, numSegments, thickness);
		released = released || ImGui::GizmoCircleResizable(worldPosition, radius, rotation * glm::quat(glm::vec3(0, glm::radians(90.f), 0)), pCamera, screenPos, screenSize, color, dragSecondCircle, numSegments, thickness);

		return released;
	}

	void GizmoCircle2D(const glm::vec3& worldPosition, float radius, const sa::SceneCamera* pCamera, const glm::vec2& screenPos, const glm::vec2& screenSize, const ImColor& color, int numSegments, float thickness) {
		glm::vec3 point = sa::math::worldToScreen(worldPosition, pCamera, screenPos, screenSize);
		glm::vec3 point2 = sa::math::worldToScreen(worldPosition + (pCamera->getRight() * radius), pCamera, screenPos, screenSize);
		if (point.z < 1.f || point2.z < 1.f)
			return;
		float radiusScreenSpace = glm::distance(glm::vec2(point), glm::vec2(point2));
		ImGui::GetWindowDrawList()->AddCircle(ImVec2(point.x, point.y), radiusScreenSpace, color, numSegments, thickness);
	}

	void GizmoCircle2DResizable(const glm::vec3& worldPosition, float& radius, const sa::SceneCamera* pCamera, const glm::vec2& screenPos, const glm::vec2& screenSize, const ImColor& color, bool& isDragging, int numSegments, float thickness) {
		glm::vec3 point = sa::math::worldToScreen(worldPosition, pCamera, screenPos, screenSize);
		glm::vec3 point2 = sa::math::worldToScreen(worldPosition + (pCamera->getRight() * radius), pCamera, screenPos, screenSize);
		if (point.z < 1.f || point2.z < 1.f)
			return;
		float radiusScreenSpace = glm::distance(glm::vec2(point), glm::vec2(point2));
		ImGui::GetWindowDrawList()->AddCircle(ImVec2(point.x, point.y), radiusScreenSpace, color, numSegments, thickness);
	
		ImVec2 rectMin(point2.x - HandleSize, point2.y - HandleSize);
		ImVec2 rectMax(point2.x + HandleSize, point2.y + HandleSize);

		ImGui::GetWindowDrawList()->AddCircleFilled({ point2.x, point2.y }, 5, color);
		ImVec2 windowPos = ImGui::GetWindowPos();
		ImGui::SetCursorPos(ImVec2(rectMin.x - windowPos.x, rectMin.y - windowPos.y));
		
		ImGui::InvisibleButton("circle_handle", ImVec2(rectMax.x - rectMin.x, rectMax.y - rectMin.y));
		if (ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
			isDragging = true;
		else if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
			isDragging = false;

		if (isDragging) {
			point2.x += ImGui::GetMouseDragDelta(ImGuiMouseButton_Left, 0.0f).x;
			ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
			float ratio = radius / radiusScreenSpace;
			radiusScreenSpace = glm::distance(point, point2);
			radius = radiusScreenSpace * ratio;
		}

	}

	void GizmoQuad(const glm::vec3& worldPosition, const glm::vec2& size, const glm::quat& rotation, const sa::SceneCamera* pCamera, const glm::vec2& screenPos, const glm::vec2& screenSize, const ImColor& color, float thickness) {
		
		glm::vec3 point1 = worldPosition + rotation * glm::vec3(size.x, size.y, 0);
		glm::vec3 point2 = worldPosition + rotation * glm::vec3(size.x, -size.y, 0);
		glm::vec3 point3 = worldPosition + rotation * glm::vec3(-size.x, -size.y, 0);
		glm::vec3 point4 = worldPosition + rotation * glm::vec3(-size.x, size.y, 0);


		glm::vec3 screenPoint1 = sa::math::worldToScreen(point1, pCamera, screenPos, screenSize);
		glm::vec3 screenPoint2 = sa::math::worldToScreen(point2, pCamera, screenPos, screenSize);
		glm::vec3 screenPoint3 = sa::math::worldToScreen(point3, pCamera, screenPos, screenSize);
		glm::vec3 screenPoint4 = sa::math::worldToScreen(point4, pCamera, screenPos, screenSize);

		if ((int)screenPoint1.z + (int)screenPoint2.z + (int)screenPoint3.z + (int)screenPoint4.z > 1) {
			return;
		}

		ImGui::GetWindowDrawList()->AddQuad(
			{ screenPoint1.x, screenPoint1.y },
			{ screenPoint2.x, screenPoint2.y },
			{ screenPoint3.x, screenPoint3.y },
			{ screenPoint4.x, screenPoint4.y },
			color, thickness);

	}

	void GizmoBox(const glm::vec3& worldPosition, const glm::vec3& halfLengths, const glm::quat& rotation, const sa::SceneCamera* pCamera, const glm::vec2& screenPos, const glm::vec2& screenSize, const ImColor& color, float thickness) {
		
		glm::vec3 dirZ = rotation * glm::vec3(0, 0, halfLengths.z);
		glm::vec3 dirX = rotation * glm::vec3(halfLengths.x, 0, 0);

		GizmoQuad(worldPosition + dirZ, halfLengths, rotation, pCamera, screenPos, screenSize, color, thickness);
		GizmoQuad(worldPosition - dirZ, halfLengths, rotation, pCamera, screenPos, screenSize, color, thickness);
		GizmoQuad(worldPosition + dirX, glm::vec2(halfLengths.z, halfLengths.y), rotation * glm::quat({ 0, glm::radians(90.f), 0 }), pCamera, screenPos, screenSize, color, thickness);
		GizmoQuad(worldPosition - dirX, glm::vec2(halfLengths.z, halfLengths.y), rotation * glm::quat({ 0, glm::radians(90.f), 0 }), pCamera, screenPos, screenSize, color, thickness);

	}

	bool GizmoBoxResizable(const glm::vec3& worldPosition, glm::vec3& halfLengths, const glm::quat& rotation, const sa::SceneCamera* pCamera, const glm::vec2& screenPos, const glm::vec2& screenSize, const ImColor& color, bool disabled, float thickness) {
		glm::vec3 dirZ = rotation * glm::vec3(0, 0, halfLengths.z);
		glm::vec3 dirX = rotation * glm::vec3(halfLengths.x, 0, 0);

		GizmoQuad(worldPosition + dirZ, halfLengths, rotation, pCamera, screenPos, screenSize, color, thickness);
		GizmoQuad(worldPosition - dirZ, halfLengths, rotation, pCamera, screenPos, screenSize, color, thickness);
		GizmoQuad(worldPosition + dirX, glm::vec2(halfLengths.z, halfLengths.y), rotation * glm::quat({ 0, glm::radians(90.f), 0 }), pCamera, screenPos, screenSize, color, thickness);
		GizmoQuad(worldPosition - dirX, glm::vec2(halfLengths.z, halfLengths.y), rotation * glm::quat({ 0, glm::radians(90.f), 0 }), pCamera, screenPos, screenSize, color, thickness);

		if (disabled)
			return false;

		glm::vec3 dirY = rotation * glm::vec3(0, halfLengths.y, 0);

		glm::vec3 pointX1 = sa::math::worldToScreen(worldPosition + dirX, pCamera, screenPos, screenSize);
		glm::vec3 pointX2 = sa::math::worldToScreen(worldPosition - dirX, pCamera, screenPos, screenSize);

		glm::vec3 pointY1 = sa::math::worldToScreen(worldPosition + dirY, pCamera, screenPos, screenSize);
		glm::vec3 pointY2 = sa::math::worldToScreen(worldPosition - dirY, pCamera, screenPos, screenSize);

		glm::vec3 pointZ1 = sa::math::worldToScreen(worldPosition + dirZ, pCamera, screenPos, screenSize);
		glm::vec3 pointZ2 = sa::math::worldToScreen(worldPosition - dirZ, pCamera, screenPos, screenSize);

		ImColor darkerColor(color.Value.x - 0.5f, color.Value.y - 0.5f, color.Value.z - 0.5f, color.Value.w - 0.5f);
		glm::vec2 pointX;
		glm::vec2 pointY;
		glm::vec2 pointZ;

		ImGui::GetWindowDrawList()->AddCircleFilled({ pointX1.x, pointX1.y }, HandleSize, (pointX1.z <= pointX2.z) ? (pointX = pointX1), color : darkerColor);
		ImGui::GetWindowDrawList()->AddCircleFilled({ pointX2.x, pointX2.y }, HandleSize, (pointX1.z > pointX2.z) ? (pointX = pointX2), color : darkerColor);

		ImGui::GetWindowDrawList()->AddCircleFilled({ pointY1.x, pointY1.y }, HandleSize, (pointY1.z <= pointY2.z) ? (pointY = pointY1), color : darkerColor);
		ImGui::GetWindowDrawList()->AddCircleFilled({ pointY2.x, pointY2.y }, HandleSize, (pointY1.z > pointY2.z) ? (pointY = pointY2), color : darkerColor);

		ImGui::GetWindowDrawList()->AddCircleFilled({ pointZ1.x, pointZ1.y }, HandleSize, (pointZ1.z <= pointZ2.z) ? (pointZ = pointZ1), color : darkerColor);
		ImGui::GetWindowDrawList()->AddCircleFilled({ pointZ2.x, pointZ2.y }, HandleSize, (pointZ1.z > pointZ2.z) ? (pointZ = pointZ2), color : darkerColor);

		bool mouseDown = ImGui::IsMouseDown(ImGuiMouseButton_Left);

		static bool isDragging[6] = { false, false, false, false, false, false };
		glm::vec2 points[6] = { pointX1, pointX2, pointY1, pointY2, pointZ1, pointZ2 };
		float* halfs[6] = { &halfLengths.x, &halfLengths.x, &halfLengths.y, &halfLengths.y, &halfLengths.z, &halfLengths.z };


		glm::vec2 mousePos = { ImGui::GetMousePos().x, ImGui::GetMousePos().y };
		glm::vec3 center = sa::math::worldToScreen(worldPosition, pCamera, screenPos, screenSize);
		for (int i = 0; i < 6; i++) {
			float distance = glm::distance(mousePos, points[i]);
			bool isOver = distance <= HandleSize;
			if(isOver)
				ImGui::GetWindowDrawList()->AddCircleFilled({ points[i].x, points[i].y }, HandleSize, ImColor(1.f, 1.f, 1.f));

			if (isOver && mouseDown) {
				isDragging[i] = true;
			}
			else if (isDragging[i] && !mouseDown) {
				isDragging[i] = false;
				return true;
			}

			if (isDragging[i]) {
				glm::vec2 halfLengthScreenSpace = glm::vec2(center) - points[i];
				float length = glm::length(halfLengthScreenSpace);
				float ratio = *halfs[i] / length;
				glm::vec2 toCenter = halfLengthScreenSpace / length;

				ImVec2 imDelta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left, 0.f);
				ImGui::ResetMouseDragDelta();
				glm::vec2 delta = { imDelta.x, imDelta.y };
				halfLengthScreenSpace -= (delta * glm::abs(toCenter));
				*halfs[i] = glm::length(halfLengthScreenSpace * ratio);

				ImGui::GetWindowDrawList()->AddCircleFilled({ points[i].x, points[i].y}, HandleSize * 1.5f, color);

				return false;
			}
		}
		return false;
	}

}
