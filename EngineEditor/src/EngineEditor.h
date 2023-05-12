#pragma once

#include <Application.h>
#include <RenderWindow.hpp>

#define USE_IMGUI_API

#include "SceneView.h"
#include "SceneHierarchy.h"
#include "EntityInspector.h"
#include "GameView.h"
#include "RenderPipelinePreferences.h"
#include "DirectoryView.h"
#include <imgui_internal.h>

namespace sa {

	class EngineEditor : public IApplicationLayer {
	private:
		inline static std::filesystem::path s_editorPath;
		
		enum class State {
			PLAYING,
			PAUSED,
			EDIT
		} m_state = State::EDIT;

		sa::Engine* m_pEngine;
		sa::RenderWindow* m_pWindow;

		std::vector<std::unique_ptr<EditorModule>> m_editorModules;

		std::filesystem::path m_projectFile;

		std::vector<std::filesystem::path> m_recentProjectPaths;
		
		Texture2D m_logoTex;
		Texture2D m_playPauseTex;

		bool m_enterSceneNamePopup;

		bool m_profilerWindow;

		void makePopups();

		bool openProject(const std::filesystem::path& path);
		bool openProject();

		bool createProject(const std::filesystem::path& path);
		bool createProject();
		bool saveProject(const std::filesystem::path& path);

		void projectSelector();
		void fileMenu();

		void startSimulation();
		void stopSimulation();

		void imGuiProfiler();

	public:
		static std::filesystem::path MakeEditorRelative(const std::filesystem::path& editorRelativePath);
		
		EngineEditor() = default;
		
		void onAttach(sa::Engine& engine, sa::RenderWindow& renderWindow) override;
		void onDetach() override;
		void onImGuiRender() override;
		void onUpdate(float dt) override;
		
		std::vector<std::filesystem::path> fetchAllScriptsInProject();

		RenderWindow* getWindow() const;

		EditorModule* getModuleByName(const char* name) const;
		const std::vector<std::unique_ptr<EditorModule>>& getModules() const;

	};
}