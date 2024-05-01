#include "CustomImGui.h"

#include "Tools\Math.h"
#include "Assets/ModelAsset.h"
#include "Assets/TextureAsset.h"
#include "Graphics/Material.h"
#include "Assets/MaterialShader.h"

#include "Tools\FileDialogs.h"

#include "imgui_internal.h"
#include "imgui_stdlib.h"

#include "EngineEditor.h"


#include <PhysicsSystem.h>

#include "Lua/Ref.h"

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

	sol::lua_value DisplayLuaValue(const std::string& keyAsStr, const sol::object& value) {
		switch (value.get_type()) {
		case sol::type::userdata:
			return DisplayLuaUserdata(keyAsStr, value.as<sol::userdata>());

		case sol::type::table:
			DisplayLuaTable(keyAsStr, value.as<sol::table>());
			break;

		case sol::type::function:
			ImGui::Text(keyAsStr.c_str());
			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("function");
			}
			break;
		case sol::type::string: {
			std::string v = value.as<std::string>();
			if (ImGui::InputText(keyAsStr.c_str(), &v, ImGuiInputTextFlags_EnterReturnsTrue))
				return v;
			
			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("string");
			}
			break;
		}
		case sol::type::number: {
			float v = value.as<float>();
			if(ImGui::InputFloat(keyAsStr.c_str(), &v))
				return v;
			
			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("number");
			}
			break;
		}
		case sol::type::boolean: {
			bool v = value.as<bool>();
			if(ImGui::Checkbox(keyAsStr.c_str(), &v))
				return v;

			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("boolean");
			}
			break;
		}
		default:
			ImGui::Text("%s = nil", keyAsStr.c_str());
			break;
		}
		return sol::nil;
	}

	sol::lua_value DisplayLuaUserdata(const std::string& keyAsStr, const sol::userdata& value) {
		if (value.is<sa::Vector2>()) {
			sa::Vector2& vec2 = value.as<sa::Vector2>();
			ImGui::DragFloat2(keyAsStr.c_str(), (float*)&vec2, 0.5f);

			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("Vec2");
			}
		}
		else if (value.is<sa::Vector3>()) {
			sa::Vector3& vec3 = value.as<sa::Vector3>();
			ImGui::DragFloat3(keyAsStr.c_str(), (float*)&vec3, 0.5f);

			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("Vec3");
			}
		}
		else if (value.is<sa::Vector4>()) {
			sa::Vector4& vec4 = value.as<sa::Vector4>();
			ImGui::ColorEdit4(keyAsStr.c_str(), (float*)&vec4, ImGuiColorEditFlags_Float);
			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("Vec4");
			}
		}
		else if (value.is<sa::Ref>()) {
			sa::Ref& ref = value.as<sa::Ref>();
			const std::string& typeStr = ref.getType();
			sol::lua_value refValue = ref.getValue();

			sa::Entity entity;
			if (refValue.is<sa::Entity>()) {
				entity = refValue.as<sa::Entity>();
			}

			sol::lua_value type = sa::LuaAccessable::getState()[typeStr];
			if (!type.is<sol::nil_t>()) { // is userdata
				sa::ComponentType componentType = sa::getComponentType(typeStr);
				if (componentType.isValid()) { // is component
					if (ComponentSlot(keyAsStr.c_str(), entity, componentType)) {
						return sa::Ref(type.as<sol::table>(), entity);
					}
				}
				else if (typeStr == "Entity") {
					if (EntitySlot(keyAsStr.c_str(), entity)) {
						return sa::Ref(type.as<sol::table>(), entity);
					}
				}
			}
			else {
				if (ScriptSlot(keyAsStr.c_str(), entity, typeStr)) {
					return sa::Ref(typeStr, entity);
				}
			}
		}
		else {
			std::string valueAsStr = sa::LuaAccessable::getState()["tostring"](value);
			ImGui::Text("%s = %s", keyAsStr.c_str(), valueAsStr.c_str());
			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("userdata");
			}
		}
		return sol::nil;
	}

	void DisplayLuaTable(const std::string& name, sol::table table) {
		bool open = ImGui::TreeNode(name.c_str());
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("table");
		}
		if (open) {	
			for (auto& [key, value] : table) {
				std::string keyAsStr = sa::LuaAccessable::getState()["tostring"](key);

				ImGui::PushID(keyAsStr.c_str());

				sol::lua_value changedValue = DisplayLuaValue(keyAsStr, value);
				if (!changedValue.is<sol::nil_t>())
					table[key] = changedValue;

				ImGui::PopID();
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
		
		sa::UUID id = model->model.getID();
		if (AssetSlot(("Model##" + entity.getComponent<comp::Name>()->name).c_str(), id, sa::AssetManager::Get().getAssetTypeID<sa::ModelAsset>())) {	
			model->model = id;
		}

		sa::Asset* pAsset = sa::AssetManager::Get().getAsset(id);
		if (pAsset && !pAsset->getProgress().isAllDone()) {
			ProgressBar(pAsset->getProgress().getAllCompletion());
		}

	}

	void Component(sa::Entity entity, comp::Light* light) {

		ImGui::ColorEdit3("Color", (float*)&light->values.color);

		ImGui::SliderFloat("Intensity", &light->values.color.a, 0.0f, 10.f);

		const char* preview = sa::to_string(light->values.type);
		if (ImGui::BeginCombo("Type", preview)) {
			for(uint32_t i = 0; i < static_cast<uint32_t>(sa::LightType::MAX_COUNT); i++) {
				sa::LightType type = static_cast<sa::LightType>(i);
				if(Selectable(sa::to_string(type), light->values.type == type)) {
					light->values.type = type;
				}
			}
			ImGui::EndCombo();
		}

		if (light->values.type == sa::LightType::POINT) {
			ImGui::SliderFloat("Attenuation radius", &light->values.position.w, 0.0, 200.f);
		}
		else if (light->values.type == sa::LightType::SPOT) {
			ImGui::SliderFloat("Attenuation range", &light->values.position.w, 0.0, 200.f);
			float cutoff = glm::degrees(light->values.direction.w);
			ImGui::SliderFloat("Cutoff Angle", &cutoff, 0.0, 360.0);
			light->values.direction.w = glm::radians(cutoff);
		}

		bool emitShadows = light->values.emitShadows;
		if (ImGui::Checkbox("Emit Shadows", &emitShadows)) {
			light->values.emitShadows = emitShadows;
		}


	}

	void Component(sa::Entity entity, comp::RigidBody* rb) {
		bool isStatic = rb->isStatic();
		if (ImGui::Checkbox("Static", &isStatic)) {
			rb->setStatic(isStatic);
		}
		bool isKinematic = rb->isKinematic();
		if (ImGui::Checkbox("Is Kinematic", &isKinematic)) {
			rb->setKinematic(isKinematic);
		}
		
		ImGui::BeginDisabled(isKinematic);
		float mass = rb->getMass();
		if (ImGui::InputFloat("Mass", &mass, 0.0f, 0.0f, "%.3f kg")) {
			rb->setMass(mass);
		}
		bool gravity = rb->isGravityEnabled();
		if (ImGui::Checkbox("Use Gravity", &gravity)) {
			rb->setGravityEnabled(gravity);
		}
		ImGui::EndDisabled();

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

		sa::Rectf rect = camera->camera.getViewport();
		ImGui::Text("Viewport");
		ImGui::Indent();
		if (ImGui::SliderFloat2("Offset##Camera", (float*)&rect.offset, 0.0f, 1.0f)) {
			camera->camera.setViewport(rect);
		}

		if (ImGui::SliderFloat2("Extent##Camera", (float*)&rect.extent, 0.0f, 1.0f)) {
			camera->camera.setViewport(rect);
		}
		ImGui::Unindent();

		const char* items[] = { "Perspective", "Orthographic" };
		const char* currentItem = items[(int)camera->camera.getProjectionMode()];
		if (ImGui::BeginCombo("Projection", currentItem)) {
			bool isSelected = currentItem == items[0];
			if (ImGui::Selectable(items[0], &isSelected)) {
				currentItem = items[0];
				camera->camera.setProjectionMode(sa::ProjectionMode::ePerspective);

			}
			isSelected = currentItem == items[1];
			if (ImGui::Selectable(items[1], &isSelected)) {
				currentItem = items[1];
				camera->camera.setProjectionMode(sa::ProjectionMode::eOrthographic);
			}

			ImGui::EndCombo();
		}

		if (currentItem == items[0]) {
			float fov = camera->camera.getFOVRadians();
			if (ImGui::SliderAngle("Fov", &fov, 10.f, 180.f)) {
				camera->camera.setFOVRadians(fov);
			}
		}
		else if (currentItem == items[1]) {

			float orthoSize = camera->camera.getOrthoWidth();
			if (ImGui::DragFloat("View Width", &orthoSize, 1.0f)) {
				camera->camera.setOrthoWidth(orthoSize);
			}
		}

		ImGui::Spacing();

		float near = camera->camera.getNear();
		if (ImGui::DragFloat("Near", &near, 0.1f, 0.0f)) {
			near = std::max(near, 0.f);
			camera->camera.setNear(near);
		}

		float far = camera->camera.getFar();
		if (ImGui::DragFloat("Far", &far, 10.f, 1.0f)) {
			camera->camera.setFar(far);
		}

		
		sa::UUID id = camera->getRenderTarget().getID();
		if (AssetSlot("RenderTarget", id, sa::AssetManager::Get().getAssetTypeID<sa::RenderTarget>())) {
			camera->setRenderTarget(sa::AssetManager::Get().getAsset<sa::RenderTarget>(id));
		}

	}

	void viewFile(std::filesystem::path filePath) {
		ImGui::Selectable(filePath.filename().generic_string().c_str());
	}

	bool viewDirectory(const std::filesystem::path& directory, std::filesystem::path& openDirectory) {
		//TODO: check permissions on directory before displaying
		/*
		auto status = std::filesystem::status(directory);
		std::filesystem::perms perm = status.permissions();
		std::filesystem::perms flags = std::filesystem::perms::owner_write | std::filesystem::perms::owner_read;
		*/

		bool opened = ImGui::TreeNodeEx(directory.filename().generic_string().c_str(), ImGuiTreeNodeFlags_OpenOnArrow);
		if (ImGui::IsItemClicked()) {
			openDirectory = directory;
		}
		bool doubleClicked = false;
		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
			doubleClicked = true;
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
		return doubleClicked;
	}

	bool Script(sa::EntityScript* pScript, bool* visable) {
		if (CollapsingHeader(pScript->name.c_str(), visable)) {
			for (auto& [key, value] : pScript->serializedData) {
				auto changedValue = ImGui::DisplayLuaValue(key, pScript->env[key]);
				if (!changedValue.is<sol::nil_t>()) {
					pScript->env[key] = changedValue;
				}

				auto pos = ImVec2(GetItemRectMax().x, GetItemRectMin().y);
				if(CloseButton(ImGui::GetID((key + "##reset_button").c_str()), pos)) {
					pScript->env[key] = sol::nil;
					pScript->serializedData.erase(key);
					return true;
				}
			}
		}
		return false;
	}
	
	bool RenderLayerPreferences(sa::ShadowRenderLayer* pLayer, sa::ShadowRenderLayer::PreferencesType& prefs) {
		bool changed = false;
		const int step = 1;
		const int resStep = 1024;
		changed |= ImGui::InputScalar("Directional Resolution", ImGuiDataType_::ImGuiDataType_U32, &prefs.directionalMapResolution, &resStep, NULL, "%u");
		prefs.directionalMapResolution = std::max(prefs.directionalMapResolution, (uint32_t)resStep);
		
		changed |= ImGui::InputScalar("Omnidirectional Resolution", ImGuiDataType_::ImGuiDataType_U32, &prefs.omniMapResolution, &resStep, NULL, "%u");
		prefs.omniMapResolution = std::max(prefs.omniMapResolution, (uint32_t)resStep);
		
		changed |= ImGui::SliderFloat("Depth Bias Constant", &prefs.depthBiasConstant, 0.0f, 10.0f);
		changed |= ImGui::SliderFloat("Depth Bias Slope", &prefs.depthBiasSlope, 0.0f, 10.0f);
		changed |= ImGui::InputScalar("Cascade Count", ImGuiDataType_::ImGuiDataType_U32, &prefs.cascadeCount, &step, NULL, "%u");
		prefs.cascadeCount = std::min(prefs.cascadeCount, prefs.MaxCascadeCount);
		changed |= ImGui::SliderFloat("Cascade Split Lambda", &prefs.cascadeSplitLambda, 0.f, 1.0f);
		
		changed |= ImGui::Checkbox("Soft shadows", &prefs.softShadows);

		changed |= ImGui::Checkbox("Show cascades", &prefs.showCascades);

		return changed;
	}

	bool RenderLayerPreferences(sa::BloomRenderLayer* pLayer, sa::BloomRenderLayer::PreferencesType& prefs) {
		bool changed = false;

		if (ImGui::DragFloat("Threshold", &prefs.threshold, 0.1f)) {
			prefs.threshold = std::max(prefs.threshold, 0.0f);
			changed = true;
		}

		if (ImGui::DragFloat("Intensity", &prefs.intensity, 0.1f)) {
			prefs.intensity = std::max(prefs.intensity, 0.0f);
			changed = true;
		}

		if (ImGui::DragFloat("Spread", &prefs.spread, 0.1f)) {
			prefs.spread = std::max(prefs.spread, 0.0f);
			changed = true;
		}

		//Tone mapping
		if (ImGui::DragFloat("Exposure", &prefs.tonemapPreferences.exposure, 0.1f)) {
			prefs.tonemapPreferences.exposure = std::max(prefs.tonemapPreferences.exposure, 0.0f);
			changed = true;
		}
		if (ImGui::DragFloat("Gamma", &prefs.tonemapPreferences.gamma, 0.1f)) {
			prefs.tonemapPreferences.gamma = std::max(prefs.tonemapPreferences.gamma, 0.0f);
			changed = true;
		}
		return changed;
	}
	
	AssetEditorInfo GetAssetInfo(sa::AssetTypeID type) {
		sa::AssetManager& am = sa::AssetManager::Get();
		const static std::unordered_map<sa::AssetTypeID, AssetEditorInfo> map = {
			{
				am.getAssetTypeID<sa::Material>(), 
				{
					.inCreateMenu = true,
					.icon = LoadEditorIcon("resources/sphere-white.png"),
					.imGuiPropertiesFn = MaterialProperties,
				}
			},
			{ 
				am.getAssetTypeID<sa::ModelAsset>(), 
				{
					.inCreateMenu = false,
					.icon = LoadEditorIcon("resources/mesh_cube.png"),
					.imGuiPropertiesFn = ModelProperties,
				}
			},
			{ 
				am.getAssetTypeID<sa::TextureAsset>(), 
				{
					.inCreateMenu = false,
					.icon = LoadEditorIcon("resources/image.png"),
					.imGuiPropertiesFn = TextureProperties,
				}
			},
			{
				am.getAssetTypeID<sa::Scene>(),
				{
					.inCreateMenu = true,
					.icon = LoadEditorIcon("resources/scene-white.png"),
					.imGuiPropertiesFn = [](sa::Asset* pAsset) {
						return false;
					}
				}
			},
			{
				am.getAssetTypeID<sa::RenderTarget>(),
				{
					.inCreateMenu = true,
					.icon = LoadEditorIcon("resources/image.png"),
					.imGuiPropertiesFn = RenderTargetProperties,
				}
			},
			{
				am.getAssetTypeID<sa::MaterialShader>(),
				{
					.inCreateMenu = true,
					.icon = LoadEditorIcon("resources/file-white.png"),
					.imGuiPropertiesFn = MaterialShaderProperties,
				}
			}
		};

		if (map.contains(type)) {
			return map.at(type);
		}

		static AssetEditorInfo info{
			.inCreateMenu = false,
			.icon = LoadEditorIcon("resources/file-white.png"),
			.imGuiPropertiesFn = [](sa::Asset*) { return false; },
		};
		return info;
	}

	sa::Texture LoadEditorIcon(const std::filesystem::path& path) {
		sa::Texture t;
		t.create2D(sa::Image(sa::EngineEditor::MakeEditorRelative(path).generic_string().c_str()), true);
		return t;
	}

	bool MaterialProperties(sa::Asset* pAsset) {
		sa::Material* pMaterial = static_cast<sa::Material*>(pAsset);

		sa::UUID id = pMaterial->getMaterialShader().getID();
		if(AssetSlot("Material Shader", id, sa::AssetManager::Get().getAssetTypeID<sa::MaterialShader>())) {
			pMaterial->setMaterialShader(id);
		}

		
		ColorEdit3("Albedo Color", (float*)&pMaterial->values.albedoColor);
		ColorEdit3("Emissive Color", (float*)&pMaterial->values.emissiveColor);
		DragFloat("Emissive Strength", &pMaterial->values.emissiveColor.a, 0.1f);
		pMaterial->values.emissiveColor.a = std::max(pMaterial->values.emissiveColor.a, 0.0f);

		SliderFloat("Opacity", &pMaterial->values.opacity, 0.0f, 1.0f);
		SliderFloat("Metallic", &pMaterial->values.metallic, 0.0f, 1.0f);
		SliderFloat("Roughness", &pMaterial->values.roughness, 0.0f, 1.0f);
		
		Checkbox("Two Sided", &pMaterial->twoSided);

		auto& textures = pMaterial->getTextures();
		sa::AssetTypeID textureAssetType = sa::AssetManager::Get().getAssetTypeID<sa::TextureAsset>();
		if (BeginListBox("Textures")) {
			for (auto& [type, texArr] : textures) {
				std::string textureTypeName = sa::to_string(type);
				int i = 0;
				for (auto& tex : texArr) {
					PushID(tex.getID() + i);
					if(SmallButton("-")) {
						texArr.erase(texArr.begin() + i);
						PopID();
						break;
					}
					SameLine();

					sa::UUID id = tex.getID();
					if (AssetSlot((textureTypeName + " Texture").c_str(), id, textureAssetType)) {
						tex = id;
						pMaterial->update();
					}
					PopID();
					i++;
				}
			}
			EndListBox();
		}

		if(BeginPopup("select_texture_type")) {

			
			for(int i = static_cast<uint32_t>(sa::MaterialTextureType::DIFFUSE); i < static_cast<uint32_t>(sa::MaterialTextureType::UNKNOWN); i++) {
				auto textureType = static_cast<sa::MaterialTextureType>(i);
				std::string textureTypeName = sa::to_string(textureType);
				if(Selectable(textureTypeName.c_str())) {
					textures[textureType].push_back(0);
					pMaterial->update();
					CloseCurrentPopup();
				}
			}

			EndPopup();
		}


		if(Button("+")) {
			OpenPopup("select_texture_type");
		}

		return false;
	}

	bool ModelProperties(sa::Asset* pAsset) {
		sa::ModelAsset* pModel = static_cast<sa::ModelAsset*>(pAsset);
		if (BeginListBox("Meshes")) {
			int i = 0;
			for (auto& mesh : pModel->data.meshes) {
				PushID(i);

				sa::UUID id = mesh.material.getID();
				if (AssetSlot("Material", id, mesh.material.GetTypeID())) {
					mesh.material = id;
				}
				PopID();
				i++;
			}
			EndListBox();
		}
		return false;
	}

	bool TextureProperties(sa::Asset* pAsset) {
		sa::TextureAsset* pTexture = static_cast<sa::TextureAsset*>(pAsset);
		ImVec2 size = GetContentRegionAvail();
		float aspect = (float)pTexture->getTexture().getExtent().height / pTexture->getTexture().getExtent().width;
		size.y = size.x * aspect;
		Image(pTexture->getTexture(), size);
		Text("Extent: %d, %d", pTexture->getTexture().getExtent().width, pTexture->getTexture().getExtent().height);
		return false;
	}

	bool RenderTargetProperties(sa::Asset* pAsset) {
		sa::RenderTarget* pRenderTarget = static_cast<sa::RenderTarget*>(pAsset);
		sa::Extent extent = pRenderTarget->getExtent();
		if (InputScalarN("Extent", ImGuiDataType_U32, (uint32_t*)&extent, 2, NULL, NULL, "%d", ImGuiInputTextFlags_EnterReturnsTrue)) {
			pRenderTarget->resize(extent);
			return true;
		}
		ImVec2 size = GetContentRegionAvail();
		float aspect = (float)pRenderTarget->getExtent().height / pRenderTarget->getExtent().width;
		size.y = size.x * aspect;

		Image(pRenderTarget->getOutputTexture(), size);

		return false;
	}

	bool MaterialShaderProperties(sa::Asset* pAsset) {
		sa::MaterialShader* pMaterialShader = static_cast<sa::MaterialShader*>(pAsset);
		ImVec2 size = ImGui::GetContentRegionAvail();
		static std::string errorMsg;

		if (ImGui::BeginListBox("Source Files", ImVec2(size.x, 0.0f))) {
			int i = 0;
			for (auto& sourceFile : pMaterialShader->getShaderSourceFiles()) {
				if (sourceFile.stage == 0)
					continue;

				ImGui::PushID(i);
				std::string preview = sa::to_string(sourceFile.stage);
				if (ImGui::BeginCombo("Stage", preview.c_str())) {
					uint32_t stage = sa::ShaderStageFlagBits::VERTEX;
					while (stage != sa::ShaderStageFlagBits::COMPUTE << 1) {
						if (ImGui::Selectable(sa::to_string((sa::ShaderStageFlagBits)stage).c_str(), sourceFile.stage & stage)) {
							sourceFile.stage = (sa::ShaderStageFlagBits)stage;
						}
						stage = stage << 1;
					}
					preview = sa::to_string(sourceFile.stage);
					ImGui::EndCombo();
				}
				std::string path = sourceFile.filePath.generic_string();
				if (ImGui::InputText(("##" + sa::to_string(sourceFile.stage)).c_str(), &path, ImGuiInputTextFlags_EnterReturnsTrue)) {
					sourceFile.filePath = path;
					if(!path.empty())
						sourceFile.filePath.replace_extension(".glsl");
				}
				if(BeginDragDropTarget()) {
					auto payload = AcceptDragDropPayload("Path");
					if(payload && payload->IsDelivery()) {
						sourceFile.filePath = *static_cast<std::filesystem::path*>(payload->Data);
					}
					EndDragDropTarget();
				}
				
				if (!std::filesystem::exists(sourceFile.filePath)) {
					ImGui::PushStyleColor(ImGuiCol_Text, IMGUI_COLOR_ERROR_RED);
					ImGui::Text("No Such File");
					ImGui::PopStyleColor();
					ImGui::SameLine();
					if (ImGui::Button("Create")) {
						if (!std::filesystem::exists(sourceFile.filePath.parent_path())) {
							SA_DEBUG_LOG_ERROR(sourceFile.filePath.parent_path(), " does not exist");
						}
						else {
							createGlslFile(sourceFile.filePath, sourceFile.stage);
							
						}
					}
				}
				else if (ImGui::Button("Open")) {
					SA_DEBUG_LOG_INFO("Opening ", sourceFile.filePath, " in external editor");
				}

				ImGui::SameLine();
				if (ImGui::SmallButton("Remove")) {
					pMaterialShader->removeShaderSourceFile(sourceFile.stage);
				}
				ImGui::Separator();
				ImGui::PopID();
					
				i++;
			}
			ImGui::EndListBox();
		}
	
		
		if (ImGui::SmallButton("Add Stage +")) {
			pMaterialShader->addShaderSourceFile({ "", sa::ShaderStageFlagBits::VERTEX });
		}

		if (ImGui::Button("Compile")) {
			try {
				errorMsg.clear();
				pMaterialShader->compileSource();
			}
			catch (const std::exception& e) {
				SA_DEBUG_LOG_ERROR(e.what());
				errorMsg = e.what();
			}
		}

		if (!errorMsg.empty()) {
			ImGui::PushStyleColor(ImGuiCol_Text, IMGUI_COLOR_ERROR_RED);
			ImGui::Text("Error: %s", errorMsg.c_str());
			ImGui::PopStyleColor();
		}


		return false;
	}


	void AssetPreview(sa::Material* pMaterial) {

	}

	void AssetPreview(sa::ModelAsset* pModel) {

	}

	bool AssetSlot(const char* label, sa::UUID& assetID, sa::AssetTypeID typeID) {
		std::string preview = "None";
		{
			sa::Asset* pAsset = sa::AssetManager::Get().getAsset(assetID);
			if (pAsset) {
				preview = pAsset->getName();
			}
		}
		bool selected = false;
		static std::string filter;
		if(BeginCombo(label, preview.c_str())) {
			std::vector<sa::Asset*> assets;
			sa::AssetManager::Get().getAssets(&assets, typeID);
			PushID(label);
			InputText("Filter", &filter, ImGuiInputTextFlags_AutoSelectAll);
			
			float width = GetItemRectSize().x;
			if (BeginChild("##asset_list", ImVec2(width, 100))) {
				if (Selectable("None", assetID == 0)) {
					assetID = 0;
					selected = true;
					filter.clear();
					CloseCurrentPopup();
				}
				for (auto asset : assets) {
					if (!filter.empty()) {
						auto lowerName = sa::utils::toLower(asset->getName());
						auto lowerFilter = sa::utils::toLower(filter);
						if (lowerName.find(lowerFilter) == std::string::npos) {
							continue;
						}
					}

					if (Selectable(asset->getName().c_str(), asset->getID() == assetID)) {
						assetID = asset->getID();
						selected = true;
						filter.clear();
						CloseCurrentPopup();
					}
				}
				EndChild();
			}
			PopID();
			EndCombo();
		}

		if (BeginDragDropTarget()) {
			const ImGuiPayload* payload = AcceptDragDropPayload("Path");
			if (payload && payload->IsDelivery()) {
				std::filesystem::path* pPath = (std::filesystem::path*)payload->Data;
				if (pPath->extension() == ".asset") {
					sa::Asset* asset = sa::AssetManager::Get().findAssetByPath(*pPath);
					if (asset && asset->getID() != assetID && asset->getType() == typeID) {
						assetID = asset->getID();
						selected = true;
					}
				}
			}
			EndDragDropTarget();
		}
		return selected;
	}

	bool FileSlot(const char* label, std::filesystem::path& path, const char* extension) {
		std::string pathStr = path.generic_string();
		if (ImGui::InputText(label, &pathStr, ImGuiInputTextFlags_EnterReturnsTrue)) {
			path = pathStr;
			if (!pathStr.empty())
				path.replace_extension(extension);
		}

		if (!std::filesystem::exists(path)) {
			ImGui::PushStyleColor(ImGuiCol_Text, IMGUI_COLOR_ERROR_RED);
			ImGui::Text("No Such File");
			ImGui::PopStyleColor();
			ImGui::SameLine();
			return false;
		}
		return true;
	}

	bool ScriptSlot(const char* label, sa::Entity& entity, const std::string& scriptName) {
		std::string preview = "None";
		if (!entity.isNull()) {
			preview = entity.toString();
		}
		preview += " (" + scriptName + ")";
		bool selected = false;

		ImGui::InputText(label, preview.data(), preview.size(), ImGuiInputTextFlags_ReadOnly);

		if (BeginDragDropTarget()) {
			const ImGuiPayload* payload = AcceptDragDropPayload("Entity");
			if (payload && payload->IsDelivery()) {
				sa::Entity* pEntityPayload = static_cast<sa::Entity*>(payload->Data);
				if(pEntityPayload->getScript(scriptName)) {
					entity = *pEntityPayload;
					selected = true;
				}
			}
			EndDragDropTarget();
		}
		return selected;
	}

	bool ComponentSlot(const char* label, sa::Entity& entity, sa::ComponentType type) {
		std::string preview = "None";
		if (!entity.isNull()) {
			preview = entity.toString();
		}
		preview += " (" + type.getName() + ")";
		bool selected = false;

		ImGui::InputText(label, preview.data(), preview.size(), ImGuiInputTextFlags_ReadOnly);

		if (BeginDragDropTarget()) {
			const ImGuiPayload* payload = AcceptDragDropPayload("Entity");
			if (payload && payload->IsDelivery()) {
				sa::Entity* pEntityPayload = static_cast<sa::Entity*>(payload->Data);
				if (pEntityPayload->getComponent(type).isValid()) {
					entity = *pEntityPayload;
					selected = true;
				}
			}
			EndDragDropTarget();
		}
		return selected;
	}

	bool EntitySlot(const char* label, sa::Entity& entity) {
		std::string preview = "None";
		if (!entity.isNull()) {
			preview = entity.toString();
		}

		bool selected = false;

		ImGui::InputText(label, preview.data(), preview.size(), ImGuiInputTextFlags_ReadOnly);

		if (BeginDragDropTarget()) {
			const ImGuiPayload* payload = AcceptDragDropPayload("Entity");
			if (payload && payload->IsDelivery()) {
				sa::Entity* pEntityPayload = static_cast<sa::Entity*>(payload->Data);
				entity = *pEntityPayload;
				selected = true;
			}
			EndDragDropTarget();
		}
		return selected;
	}

	void AddEditorModuleSettingsHandler(sa::EngineEditor* pEditor) {
		
		ImGuiSettingsHandler ini_handler;
		ini_handler.TypeName = "EditorModule";
		ini_handler.TypeHash = ImHashStr("EditorModule");
		ini_handler.ClearAllFn = NULL;
		ini_handler.ReadOpenFn = [](ImGuiContext* ctx, ImGuiSettingsHandler* handler, const char* name) -> void* {
			sa::EngineEditor* pEditor = (sa::EngineEditor*)handler->UserData;
			//Get module by name
			EditorModule* pModule = pEditor->getModuleByName(name);
			// return module pointer
			return (void*)pModule;
		};
		ini_handler.ReadLineFn = [](ImGuiContext* ctx, ImGuiSettingsHandler* handler, void* editorModule, const char* line) {
			// cast editorModule
			EditorModule* pModule = (EditorModule*)editorModule;
			// read open parameter
			int isOpen = 0;
			if (sscanf_s(line, "isOpen=%d", &isOpen) == 1) {
				// set open
				pModule->setOpen((bool)isOpen);
			}
		};
		
		ini_handler.ApplyAllFn = NULL;
		ini_handler.WriteAllFn = [](ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* outBuf) {
			sa::EngineEditor* pEditor = (sa::EngineEditor*)handler->UserData;
			const auto& modules = pEditor->getModules();
			outBuf->reserve(outBuf->size() + modules.size() * sizeof(int));
			for (auto& module : modules) {
				outBuf->appendf("[%s][%s]\n", handler->TypeName, module->getName());
				outBuf->appendf("isOpen=%d\n", module->isOpen());
				outBuf->append("\n");
			}
		};
		ini_handler.UserData = pEditor;

		AddSettingsHandler(&ini_handler);
		
	}

	void DirectoryHierarchy(const char* str_id, std::filesystem::path& directory, std::filesystem::path& openDirectory, int& iconSize, const ImVec2& size) {

		ImVec2 contentArea = size;
		if (size.x == 0.f && size.y == 0.f) {
			contentArea = ImGui::GetContentRegionAvail();
		}

		if (ImGui::BeginChild(str_id, contentArea, true)) {
			for (const auto& entry : std::filesystem::directory_iterator(directory)) {
				if (entry.is_directory()) {
					if (viewDirectory(entry.path(), openDirectory)) {
						directory = entry.path();
					}
				}
				else {
					viewFile(entry.path());
				}
			}
		}
		ImGui::EndChild();

	}

	bool PasteItems(const std::set<std::filesystem::path>& items, const std::filesystem::path& target) {
		bool wasChanged = false;
		for (auto& file : items) {
			try {
				std::filesystem::copy(file, target);
				SA_DEBUG_LOG_INFO("Pasted ", file, " to ", target);
				wasChanged = true;
			}
			catch (const std::filesystem::filesystem_error& e) {
				SA_DEBUG_LOG_ERROR(e.what(), ": ", e.path1(), " ", e.path2());
			}
		}
		return wasChanged;
	}

	bool DeleteItems(std::set<std::filesystem::path>& items) {
		bool wasChanged = false;
		for (auto& path : items) {
			try {
				if (std::filesystem::is_directory(path)) {
					std::filesystem::remove_all(path);
				}
				else {
					std::filesystem::remove(path);
				}
				SA_DEBUG_LOG_INFO("Deleted ", path);

				wasChanged = true;

			}
			catch (const std::filesystem::filesystem_error& e) {
				SA_DEBUG_LOG_ERROR(e.what(), ": ", e.path1(), " ", e.path2());
			}
		}
		items.clear();
		return wasChanged;
	}

	bool MoveItem(const std::filesystem::path& item, const std::filesystem::path& targetDirectory) {
		bool wasChanged = false;
		try {
			auto newPath = targetDirectory / item.filename();
			std::filesystem::rename(item, newPath);
			SA_DEBUG_LOG_INFO("Moved ", item.filename(), " to ", targetDirectory);
			wasChanged = true;
		}
		catch (const std::filesystem::filesystem_error& e) {
			SA_DEBUG_LOG_ERROR(e.what(), ": ", e.path1(), " ", e.path2());
		}
		return wasChanged;
	}

	bool MoveItems(const std::set<std::filesystem::path>& items, const std::filesystem::path& targetDirectory) {
		bool wasChanged = false;
		for(auto& file : items) {
			if (MoveItem(file, targetDirectory))
				wasChanged = true;
		}
		return wasChanged;
	}

	bool RenameItem(const std::filesystem::path& item, const std::filesystem::path& name) {
		bool wasChanged = false;
		try {
			std::filesystem::rename(item, name);
			SA_DEBUG_LOG_INFO("Renamed ", std::filesystem::proximate(item), " to ", std::filesystem::proximate(name));
			wasChanged = true;
		}
		catch (const std::filesystem::filesystem_error& e) {
			SA_DEBUG_LOG_ERROR(e.what(), ": ", e.path1(), " ", e.path2());
		}
		return wasChanged;
	}

	bool BeginDirectoryIcons(const char* str_id, std::filesystem::path& openDirectory, 
	                         int& iconSize, bool& wasChanged, std::filesystem::path& editedFile, std::string& editingName, 
	                         std::filesystem::path& lastSelected, std::set<std::filesystem::path>& selectedItems, std::function<void()> createMenu, const ImVec2& size)
	{
		ImVec2 contentArea = size;
		if (size.x == 0.f && size.y == 0.f) {
			contentArea = ImGui::GetContentRegionAvail();
		}

		static std::filesystem::path orignalPath = openDirectory;

		bool began = BeginChild((std::string(str_id) + "1").c_str(), contentArea, false, ImGuiWindowFlags_MenuBar);
		if(began) {
			static std::set<std::filesystem::path> copiedFiles;

			//Menu Bar
			if (BeginMenuBar()) {

				if (!std::filesystem::equivalent(openDirectory, orignalPath)) {
					if (ImGui::ArrowButton((std::string("parentDir_") + str_id).c_str(), ImGuiDir_Left)) {
						if (openDirectory.has_parent_path()) {
							openDirectory = openDirectory.parent_path();
						}
					}
					SameLine();
				}
				if (BeginDragDropTarget()) {
					const ImGuiPayload* payload = AcceptDragDropPayload("Path");
					if (payload && payload->IsDelivery()) {
						auto path = static_cast<std::filesystem::path*>(payload->Data);
						if (MoveItem(*path, openDirectory.parent_path()))
							wasChanged = true;
					}
					EndDragDropTarget();
				}

				Text(openDirectory.generic_string().c_str());
				SameLine();

				SetNextItemWidth(200);
				SliderInt("Icon size", &iconSize, 20, 200);
			}
			EndMenuBar();


			//Input Events
			if (BeginPopupContextWindow()) {


				if (BeginMenu("Create...")) {
					if (MenuItem("Folder")) {
						auto newPath = openDirectory / "New Folder";
						std::filesystem::create_directory(openDirectory / "New Folder");
						editedFile = newPath;
						editingName = "New Folder";
						wasChanged = true;
					}

					createMenu();

					EndMenu();
				}

				if (MenuItem("Rename", "F2")) {
					editedFile = lastSelected;
					editingName = lastSelected.filename().generic_string();
				}

				if (!copiedFiles.empty()) {
					if (MenuItem("Paste", "Ctrl + V")) {
						if (PasteItems(copiedFiles, openDirectory))
							wasChanged = true;
					}
				}
				if (!selectedItems.empty()) {
					if (MenuItem("Copy", "Ctrl + C")) {
						copiedFiles.clear();
						copiedFiles.insert(selectedItems.begin(), selectedItems.end());
					}

					if (MenuItem("Delete", "Del")) {
						if (DeleteItems(selectedItems))
							wasChanged = true;
						lastSelected.clear();
					}
				}

				Separator();

				if(MenuItem("Open in code editor")) {
					//TODO Get code editor executable from editor settings
					if(!sa::FileDialogs::OpenFileInTextEditor("F:/Microsoft VS Code/Code.exe", openDirectory)) {
						SA_DEBUG_LOG_ERROR("Failed to open in code editor");
					}
				}

				EndPopup();
			}

			if (IsMouseClicked(ImGuiMouseButton_Left)) {
				if (!IsKeyDown(ImGuiKey_LeftShift) && !IsKeyDown(ImGuiKey_RightShift))
					selectedItems.clear();
			}

			if (IsKeyPressed(ImGuiKey_F2) && !lastSelected.empty()) {
				editedFile = lastSelected;
				editingName = lastSelected.filename().generic_string();
			}

			if (IsKeyPressed(ImGuiKey_Delete)) {
				if (DeleteItems(selectedItems))
					wasChanged = true;
				lastSelected.clear();
			}

			if (IsKeyPressed(ImGuiKey_C) && IsKeyDown(ImGuiKey_LeftCtrl)) {
				copiedFiles.clear();
				copiedFiles.insert(selectedItems.begin(), selectedItems.end());
			}

			if (IsKeyPressed(ImGuiKey_V) && IsKeyDown(ImGuiKey_LeftCtrl)) {
				if (!copiedFiles.empty()) {
					if (PasteItems(copiedFiles, openDirectory))
						wasChanged = true;
				}
			}
		}
		return began;
	}

	bool DirectoryEntry(const std::filesystem::directory_entry& entry, int iconSize,
		std::set<std::filesystem::path>& selectedItems, std::filesystem::path& lastSelected,
		bool& wasChanged, std::filesystem::path& editedFile, std::string& editingName, const sa::Texture& icon)
	{
		// Check if selected
		bool selected = selectedItems.count(entry.path());
		ImVec2 iconSizeVec = ImVec2(iconSize, iconSize);
		// Selected highlight color
		int popCount = 0;
		if (selected) {
			ImVec4 selectedCol = GetStyleColorVec4(ImGuiCol_CheckMark);
			ImVec4 hoveredCol = selectedCol + GetStyleColorVec4(ImGuiCol_ButtonHovered) * ImVec4(0.3f, 0.3f, 0.3f, 0.3f);
			PushStyleColor(ImGuiCol_Header, selectedCol);
			PushStyleColor(ImGuiCol_HeaderHovered, hoveredCol);
			popCount = 2;
		}



		// Draw icon and label

		std::string label = entry.path().filename().generic_string();
		ImVec2 totalSize = iconSizeVec + GetStyle().ItemSpacing * 2.f;
		ImVec2 textSize = CalcTextSize(label.c_str(), 0, false, totalSize.x);
		totalSize.y += textSize.y;

		BeginGroup();

		ImVec2 cursorPos = GetCursorPos();
		if (Selectable(("##directory_entry" + label).c_str(), selected, ImGuiSelectableFlags_NoPadWithHalfSpacing, totalSize)) {
			if (!IsKeyDown(ImGuiKey_LeftShift) && !IsKeyDown(ImGuiKey_RightShift))
				selectedItems.clear();
			selectedItems.insert(entry.path());
			lastSelected = entry.path();
		}
		// Drag drop source
		static std::filesystem::path payloadPath;
		if (BeginDragDropSource()) {
			payloadPath = entry.path();

			SetDragDropPayload("Path", &payloadPath, sizeof(payloadPath));
			Image(icon, iconSizeVec);
			SameLine();
			Text("%s", payloadPath.filename().generic_string().c_str());
			EndDragDropSource();
		}
		// And possibly target
		if (entry.is_directory()) {
			if (BeginDragDropTarget()) {
				const ImGuiPayload* payload = AcceptDragDropPayload("Path");
				if (payload && payload->IsDelivery()) {
					auto path = static_cast<std::filesystem::path*>(payload->Data);
					const auto& thisPath = entry.path();
					if (MoveItem(*path, thisPath))
						wasChanged = true;
				}
				EndDragDropTarget();
			}
		}


		cursorPos.x += GetStyle().ItemSpacing.x;
		SetCursorPos(cursorPos);
		// Icon
		Image(icon, iconSizeVec);

		// Text field
		cursorPos.x = GetCursorPosX();
		if (editedFile == entry.path()) {
			cursorPos.x += GetStyle().ItemInnerSpacing.x;
			SetCursorPosX(cursorPos.x);
			SetNextItemWidth(totalSize.x - GetStyle().ItemInnerSpacing.x * 2.0f);
			PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
			if (InputText("##edit_name", &editingName, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
				//rename
				std::filesystem::path newName = (editedFile.parent_path() / editingName).replace_extension(editedFile.extension());
				if (RenameItem(editedFile, newName))
					wasChanged = true;
				editedFile.clear();
			}
			if (IsMouseClicked(ImGuiMouseButton_Left) && !IsItemHovered()) {
				editedFile.clear();
			}
			else {
				ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
			}
			PopStyleVar();
		}
		else {
			cursorPos.x += (totalSize.x - textSize.x) * 0.5f;
			SetCursorPosX(cursorPos.x);
			PushTextWrapPos(cursorPos.x + textSize.x);
			TextWrapped("%s", label.c_str());
			PopTextWrapPos();
		}

		EndGroup();

		PopStyleColor(popCount);

		return false;
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

	bool ProjectButton(const char* name, const char* path, bool* isOpen) {
		if (!*isOpen)
			return false;

		ImVec2 buttonSize(GetWindowContentRegionWidth(), 50);
		
		ImVec2 btnMin = GetCursorPos() + GetWindowPos();
		ImVec2 btnMax = btnMin + buttonSize;

		bool isHovered = IsMouseHoveringRect(btnMin, btnMax);
		
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

		BeginChild(path, buttonSize, false);
		
		ImGuiID id = GetCurrentWindow()->GetID(path);
		ImVec2 pos = GetItemRectMin();
		auto closeBtnSize = ImVec2(GetFontSize(), GetFontSize()) + GetStyle().FramePadding * 2.0f;
		pos.x = GetItemRectMax().x - closeBtnSize.x;
		pos.x -= GetStyle().FramePadding.x;
		pos.y += buttonSize.y * 0.5f - closeBtnSize.y * 0.5f;

		if (CloseButton(id, pos)) {
			*isOpen = false;
		}
		
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

	bool TextButton(const char* label, bool center) {
		ImVec2 pos = ImGui::GetCursorPos();
		ImVec2 size = ImGui::CalcTextSize(label);

		if (center) {
			pos = { pos.x - size.x * 0.5f, pos.y - size.y * 0.5f };
			ImGui::SetCursorPos(pos);
		}
		bool pressed = ImGui::InvisibleButton(label, size);
		bool isHovered = ImGui::IsItemHovered();
		if (isHovered) {
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
		}
		ImGui::SetCursorPos(pos);
		ImGui::Text(label);
		if (isHovered) {
			ImGui::PopStyleColor();
		}

		return pressed;
	}

	void GizmoIcon(const sa::Texture* pTex, const glm::vec3& worldPoint, const sa::SceneCamera* pCamera, const glm::vec2& rectPos, const glm::vec2& rectSize, int iconSize, ImColor tintColor) {
		ImVec2 windowPos = ImGui::GetWindowPos();
		glm::vec3 point = sa::math::worldToScreen(worldPoint, pCamera, rectPos, rectSize);
		if (point.z < 1) {
			
			iconSize /= glm::distance(worldPoint, pCamera->getPosition());
			ImGui::GetWindowDrawList()->AddImageQuad(
				sa::Renderer::Get().getImGuiTexture(pTex),
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
				ImVec2 windowPos = ImGui::GetWindowPos();
				glm::vec2 windowPoint = { screenPoint.x - windowPos.x, screenPoint.y - windowPos.y };

				ImVec2 rectMin(screenPoint.x - HandleSize, screenPoint.y - HandleSize);
				ImVec2 rectMax(screenPoint.x + HandleSize, screenPoint.y + HandleSize);
				ImGui::GetWindowDrawList()->AddCircleFilled({ screenPoint.x, screenPoint.y }, HandleSize, color);

				bool isOver = ImGui::IsMouseHoveringRect(rectMin, rectMax);

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
		if (point.z > 1.f || point2.z > 1.f)
			return;
		float radiusScreenSpace = glm::distance(glm::vec2(point), glm::vec2(point2));
		if (radiusScreenSpace > 0.0001f)
			ImGui::GetWindowDrawList()->AddCircle(ImVec2(point.x, point.y), radiusScreenSpace, color, numSegments, thickness);
	}

	void GizmoCircleFilled2D(const glm::vec3& worldPosition, float radius, const sa::SceneCamera* pCamera, const glm::vec2& screenPos, const glm::vec2& screenSize, const ImColor& color, int numSegments) {
		glm::vec3 point = sa::math::worldToScreen(worldPosition, pCamera, screenPos, screenSize);
		glm::vec3 point2 = sa::math::worldToScreen(worldPosition + (pCamera->getRight() * radius), pCamera, screenPos, screenSize);
		if (point.z > 1.f || point2.z > 1.f)
			return;
		float radiusScreenSpace = glm::distance(glm::vec2(point), glm::vec2(point2));
		if (radiusScreenSpace > 0.0001f)
			ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(point.x, point.y), radiusScreenSpace, color, numSegments);
	}

	void GizmoCircle2DResizable(const glm::vec3& worldPosition, float& radius, const sa::SceneCamera* pCamera, const glm::vec2& screenPos, const glm::vec2& screenSize, const ImColor& color, bool& isDragging, int numSegments, float thickness) {
		glm::vec3 point = sa::math::worldToScreen(worldPosition, pCamera, screenPos, screenSize);
		glm::vec3 point2 = sa::math::worldToScreen(worldPosition + (pCamera->getRight() * radius), pCamera, screenPos, screenSize);
		if (point.z > 1.f || point2.z > 1.f)
			return;
		float radiusScreenSpace = glm::distance(glm::vec2(point), glm::vec2(point2));
		if (radiusScreenSpace > 0.0001f)
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
