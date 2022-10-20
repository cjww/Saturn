#pragma once

#include <Application.h>
#include <RenderWindow.hpp>

#define USE_IMGUI_API

#include "SceneView.h"
#include "SceneHierarchy.h"
#include "EntityInspector.h"
#include "LuaConsole.h"

#define SA_PROJECT_FILE_EXT ".saproj"

namespace sa {

	class EngineEditor : public IApplicationLayer {
	private:
		sa::Engine* m_pEngine;
		std::vector<std::unique_ptr<EditorModule>> m_editorModules;
		std::filesystem::path m_projectPath;
		
		void projectSelector();

	public:
		EngineEditor() = default;
		
		bool openProject(const std::filesystem::path& projectPath);

		void onAttach(sa::Engine& engine, sa::RenderWindow& renderWindow) override;
		void onDetach() override;
		void onImGuiRender() override;
		void onUpdate(float dt) override;
	};
}