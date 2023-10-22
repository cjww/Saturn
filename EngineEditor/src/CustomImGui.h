#pragma once



#include "ECS/Components.h"
#include "ECS/Entity.h"
#include "SceneCamera.h"

#include "AssetManager.h"

#include "FileTemplates.h"

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
	sa::Texture2D icon;
	std::function<bool(sa::IAsset*)> imGuiPropertiesFn;
};

namespace ImGui {
	inline struct PopupPayload {
		sa::ComponentType type;
		std::string name;
	} payload;

	const int HandleSize = 5;

	void SetupImGuiStyle();

	sol::lua_value DisplayLuaValue(const std::string& keyAsStr, const sol::object& value);
	void DisplayLuaTable(const std::string& name, sol::table table);

	void Component(sa::Entity entity, comp::Transform* transform);
	void Component(sa::Entity entity, comp::Model* model);
	void Component(sa::Entity entity, comp::Script* script);
	void Component(sa::Entity entity, comp::Light* light);
	void Component(sa::Entity entity, comp::RigidBody* rb);
	void Component(sa::Entity entity, comp::BoxCollider* bc);
	void Component(sa::Entity entity, comp::SphereCollider* sc);
	void Component(sa::Entity entity, comp::Camera* camera);

	template<typename T>
	void Component(const sa::Entity& entity);

	void Script(sa::EntityScript& pScript, bool* visable);


	AssetEditorInfo GetAssetInfo(sa::AssetTypeID type);
	sa::Texture2D LoadEditorIcon(const std::filesystem::path& path);

	bool MaterialProperties(sa::IAsset* pAsset);
	bool ModelProperties(sa::IAsset* pAsset);
	bool TextureProperties(sa::IAsset* pAsset);
	bool RenderTargetProperties(sa::IAsset* pAsset);
	bool MaterialShaderProperties(sa::IAsset* pAsset);

	void AssetPreview(sa::Material* pMaterial);
	void AssetPreview(sa::ModelAsset* pModel);

	bool AssetSlot(const char* label, sa::IAsset*& pAsset, sa::AssetTypeID typeID);
	bool FileSlot(const char* label, std::filesystem::path& path, const char* extension);
	bool ScriptSlot(const char* label, sa::Entity& entity, const std::string& scriptName);
	bool ComponentSlot(const char* label, sa::Entity& entity, sa::ComponentType type);
	bool EntitySlot(const char* label, sa::Entity& entity);


	void AddEditorModuleSettingsHandler(sa::EngineEditor* pEditor);

	void DirectoryHierarchy(const char* str_id, const std::filesystem::path& directory, std::filesystem::path& openDirectory, int& iconSize, const ImVec2& size = ImVec2(0, 0));
	
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

