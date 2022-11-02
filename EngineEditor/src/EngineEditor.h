#pragma once

#include <Application.h>
#include <RenderWindow.hpp>

#define USE_IMGUI_API

#include "SceneView.h"
#include "SceneHierarchy.h"
#include "EntityInspector.h"

namespace sa {

	class EngineEditor : public IApplicationLayer {
	private:
		sa::Engine* m_pEngine;
		std::vector<std::unique_ptr<EditorModule>> m_editorModules;
		std::filesystem::path m_projectPath;
		
		std::vector<std::filesystem::path> m_recentProjectPaths;
		std::unordered_map<Scene*, std::filesystem::path> m_savedScenes;

		bool openProject(const std::filesystem::path& path);
		bool openProject();

		bool createProject(const std::filesystem::path& path);
		bool createProject();
		bool saveProject(const std::filesystem::path& path);

		void projectSelector();
		void fileMenu();

		void saveScene(Scene* pScene);
		void loadScene(Scene* pScene);

		std::filesystem::path makeProjectRelative(const std::filesystem::path& path);
		std::filesystem::path makeEditorRelative(const std::filesystem::path& projectRelativePath);


	public:
		EngineEditor() = default;
		
		void onAttach(sa::Engine& engine, sa::RenderWindow& renderWindow) override;
		void onDetach() override;
		void onImGuiRender() override;
		void onUpdate(float dt) override;
	};
}