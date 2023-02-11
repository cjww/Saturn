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

namespace sa {

	class EngineEditor : public IApplicationLayer {
	private:
		enum class State {
			PLAYING,
			PAUSED,
			EDIT
		} m_state = State::EDIT;

		sa::Engine* m_pEngine;
		sa::RenderWindow* m_pWindow;

		std::vector<std::unique_ptr<EditorModule>> m_editorModules;
		
		std::filesystem::path m_projectFile;
		std::filesystem::path m_editorPath;

		std::vector<std::filesystem::path> m_recentProjectPaths;
		
		Texture2D m_logoTex;
		Texture2D m_playPauseTex;

		bool m_enterSceneNamePopup;

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
		EngineEditor() = default;
		
		void onAttach(sa::Engine& engine, sa::RenderWindow& renderWindow) override;
		void onDetach() override;
		void onImGuiRender() override;
		void onUpdate(float dt) override;
		
		std::filesystem::path editorRelativePath(const std::filesystem::path& editorRelativePath);
		std::vector<std::filesystem::path> fetchAllScriptsInProject();

		RenderWindow* getWindow() const;

	};
}