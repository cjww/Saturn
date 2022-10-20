#include "EngineEditor.h"

#include <Tools\Clock.h>

#include "TestLayer.h"

#include "Tools\FileDialogs.h"

namespace sa {
	void EngineEditor::projectSelector() {
		ImGui::BeginPopupModal("ProjectName");

		if (ImGui::Begin("Select Project")) {
			
			if (ImGui::Button("Open Project...")) {
				if (FileDialogs::OpenFile("Saturn Project File (*.saproj)\0*.saproj\0", m_projectPath, std::filesystem::current_path())) {
					std::cout << "Opened Project: " << m_projectPath << std::endl;
					// open Project file
					
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("New Project +")) {
				ImGui::OpenPopup("Project Name");
			}

			static std::string name;
			if (ImGui::MakeEnterNameModalPopup("Project Name", "New Project", name)) {
				std::cout << "Created new Project: " << name << std::endl;
				//TODO create project folder and project file
			}

		}
		ImGui::End();
	}


	void EngineEditor::onAttach(sa::Engine& engine, sa::RenderWindow& renderWindow) {
		m_pEngine = &engine;

		m_editorModules.push_back(std::make_unique<SceneView>(&engine, &renderWindow));

		m_editorModules.push_back(std::make_unique<EntityInspector>(&engine));

		m_editorModules.push_back(std::make_unique<SceneHierarchy>(&engine));

		m_editorModules.push_back(std::make_unique<LuaConsole>(&engine));

		Application::get()->pushLayer(new TestLayer);

	}

	void EngineEditor::onDetach() {
		m_editorModules.clear();
	}

	void EngineEditor::onImGuiRender() {
		ImGuizmo::BeginFrame();
		if (m_projectPath.empty()) {
			projectSelector();
			return;
		}

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