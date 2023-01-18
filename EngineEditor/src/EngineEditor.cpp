#include "EngineEditor.h"

#include <Tools\Clock.h>

#include "TestLayer.h"

#include "Tools\FileDialogs.h"

#include <simdjson.h>

namespace sa {
	void EngineEditor::makePopups() {
		static std::string name;

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
			if (ImGui::Button("Create") || pressedEnter) {
				if (!name.empty()) {
					m_pEngine->setScene(name);
					ImGui::CloseCurrentPopup();
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
				erromsg = "";
				setFocus = true;
			}
			if (!erromsg.empty()) {
				ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), erromsg.c_str());
			}
			ImGui::EndPopup();
		}

		if (ImGui::BeginPopupModal("About")) {
			ImVec2 windowSize = ImGui::GetContentRegionAvail();
			ImVec2 imageSize = ImVec2(windowSize.x * 0.5f, windowSize.y * 0.5f);
			ImVec2 cursorStartPos = ImGui::GetCursorStartPos();
			ImGui::SetCursorPos(ImVec2(cursorStartPos.x + imageSize.x * 0.5f, cursorStartPos.y + imageSize.y * 0.5f));
			ImGui::Image(m_logoTex, imageSize, ImVec2(0, 0), ImVec2(1, 1), ImVec4(1.f, 1.f, 1.f, 0.2f));

			ImGui::SetCursorPos(cursorStartPos);
			ImVec2 textSize = ImGui::CalcTextSize("Saturn");
			ImGui::SetCursorPosX(windowSize.x * 0.5f - textSize.x * 0.5f);
			ImGui::Text("Saturn");

			ImGui::Spacing();

			ImGui::Text("Version: %s", SA_VERSION);
			ImGui::Text("Backend Graphics API: Vulkan 1.3");
			ImGui::Text("Physics engine: NVIDIA PhysX");

			ImGui::Spacing();
			if (ImGui::Button("Close")) {
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}

	bool EngineEditor::openProject(const std::filesystem::path& path) {
		//unload project
		m_savedScenes.clear();
		m_pEngine->destroyScenes();
		AssetManager::get().clear();
		m_pEngine->publish<editor_event::EntityDeselected>();

		using namespace simdjson;
		ondemand::parser parser;
		auto json = padded_string::load(path.string());
		if (json.error()) {
			return false;
		}
		ondemand::document doc = parser.iterate(json);
		
		auto version = doc["version"];
		if (version != SA_VERSION) {
			SA_DEBUG_LOG_ERROR("Project version missmatch:\n\tProject: ", version, "\n\tEngine: ", SA_VERSION);
		}
		SA_DEBUG_LOG_INFO("Opened Project: ", path);
		
		m_projectPath = path;

		auto scenesArray = doc["scenes"];
		for (const auto& scenePath : scenesArray) {
			simdjson::ondemand::value s = scenePath.value_unsafe();
			std::filesystem::path projectRealtiveScene(s.get_string().value());
			Scene& scene = m_pEngine->loadSceneFromFile(makeEditorRelative(projectRealtiveScene));
			m_savedScenes[&scene] = projectRealtiveScene;
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

		s.value("version", SA_VERSION);
		time_t t = time(NULL);
		s.value("last_saved", t);

		s.beginArray("scenes");

		for (const auto& [pScene, scenePath] : m_savedScenes) {
			s.value(makeProjectRelative(scenePath).generic_string().c_str());
		}

		s.endArray();

		s.endObject();

		projectFile << s.dump();
		projectFile.close();
		return true;
	}


	void EngineEditor::projectSelector() {

		ImGui::OpenPopup("Select Project");

		ImVec2 windowPos = ImGui::GetMainViewport()->Pos;
		ImVec2 windowSize = ImGui::GetMainViewport()->Size;

		ImVec2 popupSize = ImVec2(windowSize.x * 0.5f, windowSize.y * 0.5f);
		ImGui::SetNextWindowSize(popupSize, ImGuiCond_Always);
		ImGui::SetNextWindowPos(ImVec2(
			windowPos.x + (windowSize.x * 0.5f - popupSize.x * 0.5f),
			windowPos.y + (windowSize.y * 0.5f - popupSize.y * 0.5f)), ImGuiCond_Always);

		
		if (ImGui::BeginPopupModal("Select Project", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings)) {
			ImGui::Text("Recent Projects");

			if (ImGui::BeginChild("recent_projects", ImVec2(ImGui::GetWindowContentRegionWidth(), popupSize.y * 0.8f), true)) {
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

		if (ImGui::MenuItem("Save Scene", "Ctrl + S")) {
			saveScene(m_pEngine->getCurrentScene());
		}

		if (ImGui::MenuItem("Reload Scene", "Ctrl + R")) {
			loadScene(m_pEngine->getCurrentScene());
		}

		if (ImGui::MenuItem("Open Scene")) {
			openScene();
		}


	}

	bool EngineEditor::saveScene(Scene* pScene) {
		auto it = m_savedScenes.find(pScene);
		if (it == m_savedScenes.end()) {
			std::filesystem::path path;
			if (!FileDialogs::SaveFile("Saturn Scene (*.json)\0*.json\0", path, m_projectPath.parent_path())) {
				return false;
			}
			it = m_savedScenes.insert({ pScene, path }).first;
		}

		m_pEngine->storeSceneToFile(pScene, makeEditorRelative(it->second));
		return true;
	}

	bool EngineEditor::loadScene(Scene* pScene) {
		auto it = m_savedScenes.find(pScene);
		if (it == m_savedScenes.end()) {
			std::filesystem::path path;
			if (!FileDialogs::OpenFile("Saturn Scene (*.json)\0*.json\0", path, m_projectPath.parent_path())) {
				return false;
			}
			it = m_savedScenes.insert({ pScene, path }).first;
		}

		m_pEngine->loadSceneFromFile(makeEditorRelative(it->second));
		return true;
	}

	bool EngineEditor::openScene() {
		std::filesystem::path path;
		if (!FileDialogs::OpenFile("Saturn Scene (*.json)\0*.json\0", path, m_projectPath.parent_path())) {
			return false;
		}
		openScene(path);
		return true;
	}

	void EngineEditor::openScene(const std::filesystem::path& path) {
		Scene& scene = m_pEngine->loadSceneFromFile(makeEditorRelative(path));
		m_pEngine->publish<editor_event::EntityDeselected>();
		m_pEngine->setScene(scene);
	}

	void EngineEditor::startSimulation() {
		m_state = State::PLAYING;
		m_pEngine->storeSceneToFile(m_pEngine->getCurrentScene(), "sceneCache.json");
		m_pEngine->getCurrentScene()->onRuntimeStart();
	}

	void EngineEditor::stopSimulation() {
		m_pEngine->getCurrentScene()->onRuntimeStop();
		m_pEngine->loadSceneFromFile("sceneCache.json");
		m_state = State::EDIT;
	}

	std::filesystem::path EngineEditor::makeProjectRelative(const std::filesystem::path& editorRelativePath) {
		return std::filesystem::proximate(editorRelativePath, m_projectPath.parent_path());
	}

	std::filesystem::path EngineEditor::makeEditorRelative(const std::filesystem::path& projectRelativePath) {
		return m_projectPath.parent_path() / projectRelativePath;
	}

	

	void EngineEditor::onAttach(sa::Engine& engine, sa::RenderWindow& renderWindow) {
		m_pEngine = &engine;
		
		ImGui::SetupImGuiStyle();
		engine.on<engine_event::WindowResized>([](const engine_event::WindowResized& e, Engine& engine) {
			ImGui::SetupImGuiStyle();
		});

		//hijack sceneSet event
		engine.clear<engine_event::SceneSet>();
		engine.on<engine_event::SceneSet>([&](engine_event::SceneSet& e, Engine&) {
			if (m_state == State::EDIT)
				return;
			if (e.oldScene) {
				e.oldScene->onRuntimeStop();
			}
			e.newScene->onRuntimeStart();
		});


		m_editorModules.push_back(std::make_unique<SceneView>(&engine, this, &renderWindow));

		m_editorModules.push_back(std::make_unique<EntityInspector>(&engine, this));

		m_editorModules.push_back(std::make_unique<SceneHierarchy>(&engine, this));

		m_editorModules.push_back(std::make_unique<GameView>(&engine, this, &renderWindow));

		//Application::get()->pushLayer(new TestLayer);

		Image logo("resources/Logo-white.png");
		m_logoTex = Texture2D(logo, true);

		Image playPauseButtons("resources/play-pause-buttons.png");
		m_playPauseTex = Texture2D(playPauseButtons, true);

		// read recent projects
		std::ifstream recentProjectsFile("recent_projects.txt");
		std::string line;
		while (!recentProjectsFile.eof()) {
			std::getline(recentProjectsFile, line);
			if (!line.empty())
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

		bool enterSceneNamePopup = false;

		const bool isPlaying = m_state == State::PLAYING;
		const bool isPaused = m_state == State::PAUSED;

		const int buttonSize = 28;
		const int framePaddingY = 12;


		
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, framePaddingY));
		if(isPlaying || isPaused)
			ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(0.3f, 0.6f, 0.3f, 1.f));
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::ImageButtonTinted(m_logoTex, ImVec2(buttonSize + 10, buttonSize + 10))) {
				ImGui::OpenPopup("About");
			}

			//Key bindings
			if (m_state == State::EDIT) {
				if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl)) {
					if (ImGui::IsKeyPressed(ImGuiKey_S, false)) {
						saveScene(m_pEngine->getCurrentScene());
					}
				
					if (ImGui::IsKeyPressed(ImGuiKey_R, false)) {
						loadScene(m_pEngine->getCurrentScene());
					}

				}
			}

			ImGui::BeginDisabled(isPlaying || isPaused);
			if (ImGui::BeginMenu("File")) {
				fileMenu();

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu(m_pEngine->getCurrentScene()->getName().c_str())) {
				for (auto& [pScene, path] : m_savedScenes) {
					if (ImGui::MenuItem(pScene->getName().c_str())) {
						openScene(path);
					}
				}

				ImGui::Separator();
				if (ImGui::MenuItem("New Scene + ")) {
					enterSceneNamePopup = true;
				}
				ImGui::EndMenu();
			}
			ImGui::EndDisabled();

			if (enterSceneNamePopup) {
				ImGui::OpenPopup("Create New Scene");
			}

			ImGui::SetCursorPosY(framePaddingY - (buttonSize * 0.25f));
			ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 0.5f - buttonSize * 0.5f);

			float oneThird = 1 / 3.f;
			int imageIndex = 0;
			if (m_state == State::PLAYING) imageIndex = 1;
			
			// Play / pause Button
			if(ImGui::ImageButtonTinted(m_playPauseTex, ImVec2(buttonSize, buttonSize), ImVec2(imageIndex * oneThird, 0), ImVec2((1 + imageIndex) * oneThird, 1))) {
				if (isPlaying) m_state = State::PAUSED;
				else if (isPaused) m_state = State::PLAYING;
				else if (m_state == State::EDIT) {
					startSimulation();
				}

			}
			if (isPlaying || isPaused) {
				// Stop button
				ImGui::SetCursorPosY(framePaddingY - (buttonSize * 0.25f));
				if (ImGui::ImageButtonTinted(m_playPauseTex, ImVec2(buttonSize, buttonSize), ImVec2(2 * oneThird, 0), ImVec2(1, 1))) {
					stopSimulation();
				}
			}

		}
		ImGui::PopStyleVar();
		if(isPlaying || isPaused)
			ImGui::PopStyleColor();

		makePopups();

		ImGui::EndMainMenuBar();

		for (auto& module : m_editorModules) {
			module->onImGui();
		}
	}

	void EngineEditor::onUpdate(float dt) {
		if (m_state == State::PLAYING) {
			m_pEngine->getCurrentScene()->runtimeUpdate(dt);
		}
		else {
			m_pEngine->getCurrentScene()->inEditorUpdate(dt);
		}


		for (auto& module : m_editorModules) {
			module->update(dt);
		}
	}

	std::vector<std::filesystem::path> EngineEditor::fetchAllScriptsInProject() {
		std::vector<std::filesystem::path> paths;
		for (const auto& entry : std::filesystem::recursive_directory_iterator(makeEditorRelative("Assets"))) {
			if (entry.is_regular_file()) {
				std::filesystem::path path = entry;
				if (path.extension() == ".lua") {
					paths.push_back(path);
				}
			}
		}
		return std::move(paths);
	}
}