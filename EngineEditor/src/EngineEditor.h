#pragma once

#include <Application.h>
#include <RenderWindow.hpp>

#include "EditorView.h"
#include "SceneView.h"
#include "EntityInspector.h"

namespace sa {

	class EngineEditor : public IApplicationLayer {
	private:
		std::vector<std::unique_ptr<EditorModule>> m_editorModules;
	public:
		EngineEditor() = default;
		
		void onAttach(sa::Engine& engine, sa::RenderWindow& renderWindow) override;
		void onDetach() override;
		void onImGuiRender() override;
		void onUpdate(float dt) override;
	};
}