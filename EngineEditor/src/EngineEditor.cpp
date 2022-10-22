#include "EngineEditor.h"

#include <Tools\Clock.h>

#include "TestLayer.h"

#include "Tools\FileDialogs.h"

#include <simdjson.h>

namespace sa {
	bool EngineEditor::openProject(const std::filesystem::path& path) {
		using namespace simdjson;
		ondemand::parser parser;
		auto json = padded_string::load(path.string());
		if (json.error()) {
			return false;
		}
		ondemand::document doc = parser.iterate(json);

		std::cout << "Opened Project: " << path << std::endl;
		std::cout << doc["version"] << std::endl;

		m_projectPath = path;

		auto it = std::find(m_recentProjectPaths.begin(), m_recentProjectPaths.end(), m_projectPath);
		if (it != m_recentProjectPaths.end()) {
			m_recentProjectPaths.erase(it);
		}
		m_recentProjectPaths.push_back(m_projectPath);

		return true;
	}

	bool EngineEditor::openProject() {
		std::filesystem::path path;
		if (FileDialogs::OpenFile("Saturn Project File (*.saproj)\0*.saproj\0", path, std::filesystem::current_path())) {
			return openProject(path);
		}
		return false;
	}

	bool EngineEditor::createProject(const std::filesystem::path& path) {
		if (!std::filesystem::create_directory(path))
			return false;

		if (!std::filesystem::create_directory(path / "Assets"))
			return false;

		std::filesystem::path projectName = path.filename();
		projectName.replace_extension(".saproj");

		std::ofstream projectFile(path / projectName);
		if (!projectFile) {
			return false;
		}
		
		time_t t = time(NULL);
		projectFile << "{\n\"version\" : \"1.0.0\",\n\"last_saved\" : \"";
		projectFile << t;
		projectFile << "\"\n}";

		projectFile.close();

		return true;
	}

	bool EngineEditor::newProject() {
		std::filesystem::path path;
		if (FileDialogs::SaveFile("Saturn Project\0\0", path, std::filesystem::current_path())) {
			if (!createProject(path)) {
				return false;
			}
		}
		std::filesystem::path projectName = path.filename();
		projectName.replace_extension(".saproj");
		return openProject(path / projectName);
	}

	void EngineEditor::projectSelector() {
		
		ImGui::OpenPopup("Select Project");

		if (ImGui::BeginPopupModal("Select Project", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove)) {
			ImGui::Text("Recent Projects");
			if (ImGui::BeginChild("recent_projects", ImVec2(ImGui::GetWindowContentRegionWidth(), 400), true)) {
				for (auto it = m_recentProjectPaths.rbegin(); it != m_recentProjectPaths.rend(); it++) {
					if (ImGui::ProjectButton(it->filename().replace_extension().string().c_str(), it->string().c_str())) {
						openProject(*it);
						break;
					}
				}
				ImGui::EndChild();
			}
			
			if (ImGui::Button("Open Project...")) {
				openProject();
			}
			ImGui::SameLine();
			if (ImGui::Button("+ New Project...")) {
				newProject();
			}
		}
		ImGui::EndPopup();
	}


	void EngineEditor::onAttach(sa::Engine& engine, sa::RenderWindow& renderWindow) {
		m_pEngine = &engine;

		m_editorModules.push_back(std::make_unique<SceneView>(&engine, &renderWindow));

		m_editorModules.push_back(std::make_unique<EntityInspector>(&engine));

		m_editorModules.push_back(std::make_unique<SceneHierarchy>(&engine));

		m_editorModules.push_back(std::make_unique<LuaConsole>(&engine));

		Application::get()->pushLayer(new TestLayer);


		std::ifstream recentProjectsFile("recent_projects.txt");
		std::string line;
		while (!recentProjectsFile.eof()) {
			std::getline(recentProjectsFile, line);
			if(!line.empty())
				m_recentProjectPaths.push_back(line);
		}
		recentProjectsFile.close();
	}

	void EngineEditor::onDetach() {
		std::ofstream recentProjectsFile("recent_projects.txt");
		for (const auto& path : m_recentProjectPaths) {
			recentProjectsFile << path.string() << "\n";
		}
		recentProjectsFile.close();

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