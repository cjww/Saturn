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
		std::cout << "Version: " << doc["version"] << std::endl;
		
		m_projectPath = path;

		m_savedScenes.clear();
		auto scenesArray = doc["scenes"];
		for (const auto& scenePath : scenesArray) {
			if (scenePath.error()) {
				std::cout << "ERROR" << std::endl;
				break;
			}
			simdjson::ondemand::value s = scenePath.value_unsafe();
			std::filesystem::path p(s.get_string().value());
			/*
			Scene& scene = m_pEngine->loadSceneFromFile(m_projectPath.parent_path() / p);
			m_savedScenes[&scene] = p;
			*/
		}


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

		return saveProject(path / projectName);
	}

	bool EngineEditor::createProject() {
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

	bool EngineEditor::saveProject(const std::filesystem::path& path) {
		std::ofstream projectFile(path);
		if (!projectFile) {
			return false;
		}

		Serializer s;
		s.beginObject();

		s.value("version", "1.0.0");
		time_t t = time(NULL);
		s.value("last_saved", t);

		s.beginArray("scenes");

		for (const auto& [pScene, scenePath] : m_savedScenes) {
			path.native();
			s.value(std::filesystem::relative(scenePath, path.parent_path()).generic_string().c_str());
		}

		s.endArray();

		s.endObject();

		projectFile << s.dump();
		projectFile.close();
		return true;
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
				createProject();
			}
		}
		ImGui::EndPopup();
	}


	void EngineEditor::fileMenu() {

		if (ImGui::MenuItem("Save Project")) {
			saveProject(m_projectPath);
		}

		if (ImGui::MenuItem("Open Project")) {
			openProject();
		}

		if (ImGui::MenuItem("New Project")) {
			createProject();
		}

		/*

		if (ImGui::MenuItem("New Scene")) {

		}
		*/

		if (ImGui::MenuItem("Save Scene")) {
			saveScene(m_pEngine->getCurrentScene());
		}

		if (ImGui::MenuItem("Load Scene")) {
			m_pEngine->getCurrentScene()->clearEntities();
			loadScene(m_pEngine->getCurrentScene());
		}

	}

	void EngineEditor::saveScene(Scene* pScene) {
		auto it = m_savedScenes.find(pScene);
		if (it == m_savedScenes.end()) {
			std::filesystem::path path;
			if (!FileDialogs::SaveFile("\0\0", path, m_projectPath)) {
				return;
			}
			it = m_savedScenes.insert({ pScene, path}).first;
		}

		m_pEngine->storeSceneToFile(pScene, it->second);
	}

	void EngineEditor::loadScene(Scene* pScene) {
		auto it = m_savedScenes.find(pScene);
		if (it == m_savedScenes.end()) {
			std::filesystem::path path;
			if (!FileDialogs::OpenFile("\0\0", path, m_projectPath)) {
				return;
			}
			it = m_savedScenes.insert({ pScene, path }).first;
		}

		simdjson::ondemand::parser parser;
		auto json = simdjson::padded_string::load(it->second.string());
		if (json.error()) {
			std::cout << "JSON read error: " << simdjson::error_message(json.error()) << std::endl;
		}
		else {
			simdjson::ondemand::document doc = parser.iterate(json);
			pScene->deserialize(&doc);
		}
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
			if (ImGui::BeginMenu("File")) {
				fileMenu();

				ImGui::EndMenu();
			}

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