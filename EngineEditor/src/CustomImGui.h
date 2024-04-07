#pragma once



#include "ECS/Components.h"
#include "ECS/Entity.h"
#include "SceneCamera.h"

#include "AssetManager.h"

#include "FileTemplates.h"

#include "Graphics/RenderPipeline.h"
#include "Graphics/RenderLayers/BloomRenderLayer.h"
#include "Graphics/RenderLayers/ShadowRenderLayer.h"

#include <glm\gtx\matrix_decompose.hpp>
#include <glm\gtc\quaternion.hpp>

#include <filesystem>

#define IMGUI_BUFFER_SIZE_BIG 256U
#define IMGUI_BUFFER_SIZE_NORMAL 128U
#define IMGUI_BUFFER_SIZE_SMALL 64U
#define IMGUI_BUFFER_SIZE_TINY 32U

#define IMGUI_COLOR_ERROR_RED ImVec4(1.0f, 0.2f, 0.2f, 1.0f)


namespace sa {
	class EngineEditor;

}

struct AssetEditorInfo {
	bool inCreateMenu;
	sa::Texture icon;
	std::function<bool(sa::Asset*)> imGuiPropertiesFn;
};

namespace ImGui {
	inline struct PopupPayload {
		sa::ComponentType type;
		std::string name;
	} payload;

	const int HandleSize = 5;

	void SetupImGuiStyle();

	sol::lua_value DisplayLuaValue(const std::string& keyAsStr, const sol::object& value);
	sol::lua_value DisplayLuaUserdata(const std::string& keyAsStr, const sol::userdata& value);

	void DisplayLuaTable(const std::string& name, sol::table table);

	void Component(sa::Entity entity, comp::Transform* transform);
	void Component(sa::Entity entity, comp::Model* model);
	void Component(sa::Entity entity, comp::Light* light);
	void Component(sa::Entity entity, comp::RigidBody* rb);
	void Component(sa::Entity entity, comp::BoxCollider* bc);
	void Component(sa::Entity entity, comp::SphereCollider* sc);
	void Component(sa::Entity entity, comp::Camera* camera);

	template<typename T>
	void Component(const sa::Entity& entity);
	bool Script(sa::EntityScript* pScript, bool* visable);

	bool RenderLayerPreferences(sa::ShadowRenderLayer* pLayer, sa::ShadowRenderLayer::PreferencesType& prefs);
	bool RenderLayerPreferences(sa::BloomRenderLayer* pLayer, sa::BloomRenderLayer::PreferencesType& prefs);

	template<typename T, std::enable_if_t<std::is_base_of_v<sa::BasicRenderLayer, T>, bool> = true>
	bool RenderLayerPreferences(const char* title, const sa::RenderPipeline& renderPipeline);


	AssetEditorInfo GetAssetInfo(sa::AssetTypeID type);
	sa::Texture LoadEditorIcon(const std::filesystem::path& path);

	bool MaterialProperties(sa::Asset* pAsset);
	bool ModelProperties(sa::Asset* pAsset);
	bool TextureProperties(sa::Asset* pAsset);
	bool RenderTargetProperties(sa::Asset* pAsset);
	bool MaterialShaderProperties(sa::Asset* pAsset);

	void AssetPreview(sa::Material* pMaterial);
	void AssetPreview(sa::ModelAsset* pModel);

	bool AssetSlot(const char* label, sa::UUID& assetID, sa::AssetTypeID typeID);
	bool FileSlot(const char* label, std::filesystem::path& path, const char* extension);
	bool ScriptSlot(const char* label, sa::Entity& entity, const std::string& scriptName);
	bool ComponentSlot(const char* label, sa::Entity& entity, sa::ComponentType type);
	bool EntitySlot(const char* label, sa::Entity& entity);


	void AddEditorModuleSettingsHandler(sa::EngineEditor* pEditor);

	void DirectoryHierarchy(const char* str_id, const std::filesystem::path& directory, std::filesystem::path& openDirectory, int& iconSize, const ImVec2& size = ImVec2(0, 0));

	bool PasteItems(const std::set<std::filesystem::path>& items, const std::filesystem::path& targetDirectory);
	bool DeleteItems(std::set<std::filesystem::path>& items);
	bool MoveItem(const std::filesystem::path& item, const std::filesystem::path& targetDirectory);
	bool MoveItems(const std::set<std::filesystem::path>& items, const std::filesystem::path& targetDirectory);
	bool RenameItem(const std::filesystem::path& item, const std::filesystem::path& name);

	bool BeginDirectoryIcons(const char* str_id, std::filesystem::path& openDirectory,
		int& iconSize, bool& wasChanged, std::filesystem::path& editedFile, std::string& editingName, 
		std::filesystem::path& lastSelected, std::set<std::filesystem::path>& selectedItems, std::function<void()> createMenu, const ImVec2& size = ImVec2(0, 0));

	bool DirectoryEntry(const std::filesystem::directory_entry& entry, int iconSize,
		std::set<std::filesystem::path>& selectedItems, std::filesystem::path& lastSelected,
		bool& wasChanged, std::filesystem::path& editedFile, std::string& editingName, const sa::Texture& icon);

	bool MakeEnterNameModalPopup(const char* name, const char* hint, std::string& output);

	bool ProjectButton(const char* name, const char* path, bool* isOpen);

	bool ImageButtonTinted(const sa::Texture& tex, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1));

	bool TextButton(const char* label, bool center = false);

	void GizmoIcon(const sa::Texture* pTex, const glm::vec3& worldPoint, const sa::SceneCamera* pCamera, const glm::vec2& rectPos, const glm::vec2& rectSize, int iconSize, ImColor tintColor);

	void GizmoCircle(const glm::vec3& worldPosition, float radius, const glm::quat& rotation, const sa::SceneCamera* pCamera, const glm::vec2& screenPos, const glm::vec2& screenSize, const ImColor& color, int numSegments = 0, float thickness = 1.f);
	bool GizmoCircleResizable(const glm::vec3& worldPosition, float& radius, const glm::quat& rotation, const sa::SceneCamera* pCamera, const glm::vec2& screenPos, const glm::vec2& screenSize, const ImColor& color, bool& isDragging, int numSegments = 0, float thickness = 1.f);

	bool GizmoSphereResizable(const glm::vec3& worldPosition, float& radius, const glm::quat& rotation, const sa::SceneCamera* pCamera, const glm::vec2& screenPos, const glm::vec2& screenSize, const ImColor& color, bool disabled = false, int numSegments = 0, float thickness = 1.f);


	void GizmoCircle2D(const glm::vec3& worldPosition, float radius, const sa::SceneCamera* pCamera, const glm::vec2& screenPos, const glm::vec2& screenSize, const ImColor& color, int numSegments = 0, float thickness = 1.f);
	void GizmoCircleFilled2D(const glm::vec3& worldPosition, float radius, const sa::SceneCamera* pCamera, const glm::vec2& screenPos, const glm::vec2& screenSize, const ImColor& color, int numSegments = 0);
	void GizmoCircle2DResizable(const glm::vec3& worldPosition, float& radius, const sa::SceneCamera* pCamera, const glm::vec2& screenPos, const glm::vec2& screenSize, const ImColor& color, bool& isDragging, int numSegments = 0, float thickness = 1.f);

	//void GizmoQuad(const glm::mat4& transformation, const sa::SceneCamera* pCamera, const glm::vec2& screenPos, const glm::vec2& screenSize, const ImColor& color, float thickness = 1.f);
	void GizmoQuad(const glm::vec3& worldPosition, const glm::vec2& size, const glm::quat& rotation, const sa::SceneCamera* pCamera, const glm::vec2& screenPos, const glm::vec2& screenSize, const ImColor& color, float thickness = 1.f);

	void GizmoBox(const glm::vec3& worldPosition, const glm::vec3& halfLengths, const glm::quat& rotation, const sa::SceneCamera* pCamera, const glm::vec2& screenPos, const glm::vec2& screenSize, const ImColor& color, float thickness = 1.f);
	bool GizmoBoxResizable(const glm::vec3& worldPosition, glm::vec3& halfLengths, const glm::quat& rotation, const sa::SceneCamera* pCamera, const glm::vec2& screenPos, const glm::vec2& screenSize, const ImColor& color, bool disabled = false, float thickness = 1.f);


	}

template<typename T>
void ImGui::Component(const sa::Entity& entity) {
	T* comp = entity.getComponent<T>();
	if (!comp) return;


	ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
	static bool visable = true;
	
	if (ImGui::CollapsingHeader(sa::getComponentName<T>().c_str(), &visable)) {
		ImGui::Component(entity, comp);
	}
	if (!visable) {
		ImGui::OpenPopup("Remove?");
		payload.type = sa::getComponentType<T>();
		visable = true;
	}
}

template<typename T, std::enable_if_t<std::is_base_of_v<sa::BasicRenderLayer, T>, bool>>
bool ImGui::RenderLayerPreferences(const char* title, const sa::RenderPipeline& renderPipeline) {
	T* pLayer = renderPipeline.getLayer<T>();
	if (!pLayer)
		return false;
	ImGui::PushID(title);
	bool active = pLayer->isActive();
	if (ImGui::Checkbox("##active", &active)) {
		pLayer->setActive(active);
	}

	ImGui::SameLine();
	bool changed = false;
	auto& prefs = pLayer->getPreferences();
	if (ImGui::CollapsingHeader(title)) {
		changed = ImGui::RenderLayerPreferences(pLayer, prefs);

		if (ImGui::Button("Reset")) {
			prefs = {}; // TODO read from file
			changed = true;
		}
	}
	ImGui::PopID();
	return changed;
}

