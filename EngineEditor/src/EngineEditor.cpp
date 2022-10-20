#include "EngineEditor.h"

#include <Tools\Clock.h>

#include "TestLayer.h"

namespace sa {
	void EngineEditor::projectSelector() {
		ImGui::BeginPopupModal("ProjectName");

		if (ImGui::Begin("Select Project")) {
			static std::filesystem::path directoryPath = "D:/";
			static std::filesystem::path openPath;
			static int iconSize = 50;

			//ImGui::DirectoryView("EditorProjectSelector", directoryPath, openPath, iconSize);

			//ImGui::DirectoryBrowser("EditorProjectSelector", directoryPath, openPath, iconSize);
			if (ImGui::Button("New Project +")) {
				ImGui::OpenPopup("Project Name");
			}

			static std::string name;
			if (ImGui::MakeEnterNameModalPopup("Project Name", "New Project", name)) {

			}

		}
		ImGui::End();
	}

	bool EngineEditor::openProject(const std::filesystem::path& projectPath) {
		if (std::filesystem::is_directory(projectPath)) {
			for (const auto& entry : std::filesystem::directory_iterator(projectPath)) {
				if (entry.path().extension() == SA_PROJECT_FILE_EXT) {
					m_projectPath = entry.path();
					return true;
				}
			}
		}
		else if (projectPath.extension() == SA_PROJECT_FILE_EXT) {
			m_projectPath = projectPath;
			return true;
		}

		return false;
		
	}

	void EngineEditor::onAttach(sa::Engine& engine, sa::RenderWindow& renderWindow) {
		m_pEngine = &engine;

		m_editorModules.push_back(std::make_unique<SceneView>(&engine, &renderWindow));

		m_editorModules.push_back(std::make_unique<EntityInspector>(&engine));

		m_editorModules.push_back(std::make_unique<SceneHierarchy>(&engine));

		m_editorModules.push_back(std::make_unique<LuaConsole>(&engine));

		getApp()->pushLayer(new TestLayer);

	}

	void EngineEditor::onDetach() {
		m_editorModules.clear();
	}

	void EngineEditor::onImGuiRender() {
		ImGuizmo::BeginFrame();
		/*
		if (m_projectPath.empty()) {
			projectSelector();
			return;
		}
		*/

		ImGuiID viewPortDockSpaceID = ImGui::DockSpaceOverViewport();

		if (ImGui::BeginMainMenuBar()) {
			ImGui::Text("Saturn 3");


			static bool enterSceneNamePopup = false;
			if (ImGui::BeginMenu(m_pEngine->getCurrentScene()->getName().c_str())) {
				
				auto& scenes = m_pEngine->getScenes();
				for (auto& [name, scene] : scenes) {
					if (ImGui::MenuItem(name.c_str())) {
						m_pEngine->setScene(scene);
					}
				}

				ImGui::Separator();
				if (ImGui::MenuItem("New Scene + ")) {
					enterSceneNamePopup = true;
				}
				

				ImGui::EndMenu();
			}

			static std::string name;
			if (enterSceneNamePopup) {
				ImGui::OpenPopup("Create New Scene");
				name = "";
			}

			ImGui::SetNextWindowContentSize(ImVec2(300, 100));
			if (ImGui::BeginPopupModal("Create New Scene", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings)) {

				ImGui::Text("Enter name: ");
				static bool setFocus = true;
				if (setFocus) {
					ImGui::SetKeyboardFocusHere();
					setFocus = false;
				}
				bool pressedEnter = ImGui::InputTextWithHint("Name", "New Scene", &name, ImGuiInputTextFlags_EnterReturnsTrue);
				static std::string erromsg;
				ImGui::Spacing();
				if (ImGui::Button("Create") || pressedEnter)  {
					if (!name.empty()) {
						m_pEngine->setScene(name);
						ImGui::CloseCurrentPopup();
						enterSceneNamePopup = false;
						erromsg = "";
						setFocus = true;
					}
					else {
						erromsg = "Please enter a name";
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel")) {
					ImGui::CloseCurrentPopup();
					enterSceneNamePopup = false;
					erromsg = "";
					setFocus = true;
				}
				if (!erromsg.empty()) {
					ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), erromsg.c_str());
				}
				ImGui::EndPopup();
			}
		}
		ImGui::EndMainMenuBar();



		for (auto& module : m_editorModules) {
			module->onImGui();
		}
	}

	void EngineEditor::onUpdate(float dt) {
		for (auto& module : m_editorModules) {
			module->update(dt);
		}
	}
}