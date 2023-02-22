#include "EngineEditor.h"

#include <Tools\Clock.h>

#include "TestLayer.h"

#include "Tools\FileDialogs.h"

#include <simdjson.h>

#include "AssetManager.h"
#include "Assets\ModelAsset.h"

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
					Scene* scene = AssetManager::get().createAsset<Scene>(name);
					m_pEngine->setScene(scene);
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
			float smallestDim = std::min(windowSize.x, windowSize.y);
			ImVec2 imageSize = ImVec2(smallestDim * 0.5f, smallestDim * 0.5f);
			ImVec2 cursorStartPos = ImGui::GetCursorStartPos();
			ImGui::SetCursorPos(ImVec2(cursorStartPos.x + (windowSize.x * 0.5f) - imageSize.x * 0.5f, cursorStartPos.y + (windowSize.y * 0.5f) - imageSize.y * 0.5f));
			ImGui::Image(m_logoTex, imageSize, ImVec2(0, 0), ImVec2(1, 1), ImVec4(1.f, 1.f, 1.f, 0.2f));

			ImGui::SetCursorPos(cursorStartPos);
			ImVec2 textSize = ImGui::CalcTextSize("Saturn");
			ImGui::SetCursorPosX(windowSize.x * 0.5f - textSize.x * 0.5f);
			ImGui::Text("Saturn");

			ImGui::Spacing();

			ImGui::Text("Version: %s", SA_VERSION_STR);
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

		if (!m_projectFile.empty()) {
			ImGui::SaveIniSettingsToDisk("imgui.ini");
		}

		AssetManager::get().clear();
		m_pEngine->publish<editor_event::EntityDeselected>();

		using namespace simdjson;
		ondemand::parser parser;
		auto json = padded_string::load(path.generic_string());
		if (json.error()) {
			return false;
		}
		ondemand::document doc = parser.iterate(json);
		
		auto version = doc["version"].get_uint64().value();
		if (version != SA_VERSION) {
			SA_DEBUG_LOG_ERROR("Project version missmatch:\n\tProject: ", version, "\n\tEngine: ", SA_VERSION);
		}
		SA_DEBUG_LOG_INFO("Opened Project: ", path);
		
		m_projectFile = path.filename();
		std::filesystem::current_path(path.parent_path());

		ImGui::LoadIniSettingsFromDisk("imgui.ini");

		AssetManager::get().rescanAssets();

		std::string_view startScene = doc["startScene"].get_string().value();
		char* endStr;
		UUID sceneID = strtoull(startScene.data(), &endStr, 10);
		Scene* scene = AssetManager::get().getAsset<Scene>(sceneID);
		if(scene)
			scene->load();
		m_pEngine->setScene(scene);

		std::filesystem::path projectPath = path;
		auto it = std::find(m_recentProjectPaths.begin(), m_recentProjectPaths.end(), projectPath);
		if (it != m_recentProjectPaths.end()) {
			m_recentProjectPaths.erase(it);
		}
		m_recentProjectPaths.push_back(projectPath);


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

		Scene* pCurrentScene = m_pEngine->getCurrentScene();
		UUID id = 0;
		if (pCurrentScene) {
			id = pCurrentScene->getID();
			if (pCurrentScene->write()) {
				SA_DEBUG_LOG_INFO("Saving current scene...");
			}
		}

		s.value("startScene", std::to_string(id).c_str());

		s.endObject();

		projectFile << s.dump();
		projectFile.close();

		SA_DEBUG_LOG_INFO("Saved Project to ", path);

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
					bool isOpen = true;
					if (ImGui::ProjectButton(it->filename().replace_extension().string().c_str(), it->string().c_str(), &isOpen)) {
						if (!openProject(*it)) {
							SA_DEBUG_LOG_ERROR("Failed to open project");
						}
						break;
					}
					if (!isOpen) {
						int i = std::distance(m_recentProjectPaths.rbegin(), it);
						m_recentProjectPaths.erase(m_recentProjectPaths.end() - i - 1);
						break;
					}
				}
			}
			ImGui::EndChild();

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

		if (ImGui::MenuItem("Save Project", "Ctrl + S")) {
			saveProject(m_projectFile);
		}

		if (ImGui::MenuItem("Open Project")) {
			openProject();
		}

		if (ImGui::MenuItem("New Project")) {
			createProject();
		}
		if (ImGui::MenuItem("New Scene")) {
			m_enterSceneNamePopup = true;
		}

		if (ImGui::MenuItem("Reload Scene", "Ctrl + R")) {
			m_pEngine->getCurrentScene()->load();
		}
	}

	void EngineEditor::startSimulation() {
		m_state = State::PLAYING;
		
		Scene* pScene = m_pEngine->getCurrentScene();
		auto path = pScene->getAssetPath();
		pScene->setAssetPath(editorRelativePath("sceneCache.data"));
		pScene->write();
		pScene->setAssetPath(path);

		m_pEngine->getCurrentScene()->onRuntimeStart();
	}

	void EngineEditor::stopSimulation() {
		m_pEngine->getCurrentScene()->onRuntimeStop();
		
		Scene* pScene = m_pEngine->getCurrentScene();
		auto path = pScene->getAssetPath();
		pScene->setAssetPath(editorRelativePath("sceneCache.data"));
		pScene->load(sa::AssetLoadFlagBits::FORCE_SHALLOW | sa::AssetLoadFlagBits::NO_REF);
		pScene->setAssetPath(path);
		//pScene->release(); // since loading will increase refCount, we decrease it again
		
		m_state = State::EDIT;
	}


	void EngineEditor::imGuiProfiler() {
		if (ImGui::Begin("Profiler")) {
			
			static bool isRecording = false;
			static std::string filePath = "profile_editor_result.json";
			if (isRecording) {
				if (ImGui::Button("Stop recording")) {
					SA_PROFILER_END_SESSION();
					isRecording = false;
				}
			}
			else {
				ImGui::InputText("Result file", &filePath);
				
				if(ImGui::Button("Record session...")) {
					SA_PROFILER_BEGIN_SESSION();
					isRecording = true;
				}
			}


		}
		ImGui::End();
	}

	void EngineEditor::onAttach(sa::Engine& engine, sa::RenderWindow& renderWindow) {
		m_pEngine = &engine;
		m_pWindow = &renderWindow;

		m_editorPath = std::filesystem::current_path();
		
		renderWindow.addDragDropCallback([&](int count, const char** paths) {
			m_pEngine->publish<editor_event::DragDropped>(count, paths);
		});

		ImGui::SetupImGuiStyle();
		engine.on<engine_event::WindowResized>([](const engine_event::WindowResized& e, Engine& engine) {
			ImGui::SetupImGuiStyle();
		});

		ImGui::addEditorModuleSettingsHandler(this);
		ImGui::GetIO().IniFilename = NULL; // Handle loading and saving ImGui state manually

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

		m_editorModules.push_back(std::make_unique<RenderPipelinePreferences>(&engine, this));
		
		m_editorModules.push_back(std::make_unique<DirectoryView>(&engine, this));

		//Application::get()->pushLayer(new TestLayer);

		Image logo("resources/Logo-white.png");
		m_logoTex = Texture2D(logo, true);

		Image playPauseButtons("resources/play-pause-buttons.png");
		m_playPauseTex = Texture2D(playPauseButtons, true);

		// read recent projects
		std::ifstream recentProjectsFile(m_editorPath / "recent_projects.txt");
		std::string line;
		if (recentProjectsFile) {
			while (!recentProjectsFile.eof()) {
				std::getline(recentProjectsFile, line);
				if (!line.empty())
					m_recentProjectPaths.push_back(line);
			}
			recentProjectsFile.close();
		}
	}

	void EngineEditor::onDetach() {
		if (!m_projectFile.empty())
			ImGui::SaveIniSettingsToDisk("imgui.ini");

		std::ofstream recentProjectsFile(m_editorPath / "recent_projects.txt");
		for (const auto& path : m_recentProjectPaths) {
			recentProjectsFile << path.generic_string() << "\n";
		}
		recentProjectsFile.close();

		m_editorModules.clear();
	}

	void EngineEditor::onImGuiRender() {
		ImGuizmo::BeginFrame();
		if (m_projectFile.empty()) {
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
						saveProject(m_projectFile);
					}
				
					if (ImGui::IsKeyPressed(ImGuiKey_R, false)) {
						m_pEngine->getCurrentScene()->load();
					}

				}
			}

			ImGui::BeginDisabled(isPlaying || isPaused);
			m_enterSceneNamePopup = false;
			if (ImGui::BeginMenu("File")) {
				fileMenu();
				ImGui::EndMenu();
			}
			if (m_enterSceneNamePopup) {
				ImGui::OpenPopup("Create New Scene");
			}

			if (ImGui::BeginMenu("Windows")) {

				for (auto& module : m_editorModules) {
					if (ImGui::MenuItem(module->getName())) {
						module->open();
					}
				}

				ImGui::End();
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

#if SA_PROFILER_ENABLE
		imGuiProfiler();
#endif
		
	}

	void EngineEditor::onUpdate(float dt) {
		if (m_pEngine->getCurrentScene()) {
			if (m_state == State::PLAYING) {
				m_pEngine->getCurrentScene()->runtimeUpdate(dt);
			}
			else {
				m_pEngine->getCurrentScene()->inEditorUpdate(dt);
			}
		}


		for (auto& module : m_editorModules) {
			module->update(dt);
		}
	}

	std::filesystem::path EngineEditor::editorRelativePath(const std::filesystem::path& editorRelativePath) {
		return m_editorPath / editorRelativePath;
	}

	std::vector<std::filesystem::path> EngineEditor::fetchAllScriptsInProject() {
		std::vector<std::filesystem::path> paths;
		for (const auto& entry : std::filesystem::recursive_directory_iterator("Assets")) {
			if (entry.is_regular_file()) {
				std::filesystem::path path = entry;
				if (path.extension() == ".lua") {
					paths.push_back(path);
				}
			}
		}
		return std::move(paths);
	}
	
	RenderWindow* EngineEditor::getWindow() const {
		return m_pWindow;
	}
	
	EditorModule* EngineEditor::getModuleByName(const char* name) const {
		for (auto& module : m_editorModules) {
			if (strcmp(module->getName(), name) == 0) {
				return module.get();
			}
		}
		return nullptr;
	}

	const std::vector<std::unique_ptr<EditorModule>>& EngineEditor::getModules() const {
		return m_editorModules;
	}
}