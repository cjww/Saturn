#include "DirectoryView.h"

#include "AssetManager.h"
#include "Assets/ModelAsset.h"

#include "EngineEditor.h"

#include "FileTemplates.h"

void DirectoryView::onDraggedDropped(const sa::editor_event::DragDropped& e) {
	for (uint32_t i = 0; i < e.count; i++) {
		std::filesystem::path path = e.paths[i];
		std::string extension = path.extension().generic_string();
		if (sa::ModelAsset::isExtensionSupported(extension)) {
			sa::AssetManager::get().importAsset<sa::ModelAsset>(path, m_openDirectory);
			continue;
		}
		auto pTextureAsset = sa::AssetManager::get().importAsset<sa::TextureAsset>(path, m_openDirectory);
		if(pTextureAsset)
			continue;

		SA_DEBUG_LOG_WARNING("Could not import: Unsupported extension ", extension);
	}
}

void DirectoryView::onProjectOpened(const sa::editor_event::ProjectOpened& e) {
	m_openDirectory = std::filesystem::is_directory(e.projectPath)? e.projectPath : e.projectPath.parent_path();
}

DirectoryView::DirectoryView(sa::Engine* pEngine, sa::EngineEditor* pEditor)
	: EditorModule(pEngine, pEditor, "Directory View", true)
{
	m_isOpen = false;
	m_isAssetListOpen = false;

	m_pEngine->sink<sa::editor_event::DragDropped>().connect<&DirectoryView::onDraggedDropped>(this);
	m_pEngine->sink<sa::editor_event::ProjectOpened>().connect<&DirectoryView::onProjectOpened>(this);

	
	m_directoryIcon = sa::Texture2D(
		sa::Image(m_pEditor->MakeEditorRelative("resources/folder-white.png").generic_string()),
		true);
	m_otherFileIcon = sa::Texture2D(
		sa::Image(m_pEditor->MakeEditorRelative("resources/file-white.png").generic_string()),
		true);
	m_luaScriptIcon = sa::Texture2D(
		sa::Image(m_pEditor->MakeEditorRelative("resources/lua_file-white.png").generic_string()),
		true);


	m_openDirectory = std::filesystem::current_path();
}

void DirectoryView::onImGui() {
	SA_PROFILE_FUNCTION();
	for (auto it = m_openAssetProperties.begin(); it != m_openAssetProperties.end(); it++) {
		sa::Asset* pAsset = *it;
		SA_PROFILE_SCOPE(sa::AssetManager::get().getAssetTypeName(pAsset->getType()), " Properties Window");
		bool isOpen = true;
		if(ImGui::Begin((pAsset->getName() + " Properties").c_str(), &isOpen)) {
			ImGui::GetAssetInfo(pAsset->getType()).imGuiPropertiesFn(pAsset);
			if (ImGui::Button("Apply")) {
				pAsset->write();
			}
			ImGui::SameLine();
			if (ImGui::Button("Revert")) {
				pAsset->load(sa::AssetLoadFlagBits::FORCE_SHALLOW | sa::AssetLoadFlagBits::NO_REF);
			}
		}
		ImGui::End();
		if (!isOpen) {
			m_openAssetProperties.erase(it);
			pAsset->release();
			break;
		}
	}


	if (!m_isOpen)
		return;
	
	if (ImGui::Begin(m_name, &m_isOpen, ImGuiWindowFlags_MenuBar)) {
		
		if (ImGui::BeginMenuBar()) {
			if (ImGui::MenuItem("Assets Window")) {
				m_isAssetListOpen = !m_isAssetListOpen;
			}
			ImGui::EndMenuBar();
		}

		
		static int iconSize = 45;
		
		static std::filesystem::path lastSelected;
		static std::set<std::filesystem::path> selectedItems;
		static std::string editingName;
		static std::filesystem::path editedFile;

		bool wasChanged = false;

		auto menuItemsFn = [&]() {
			
			if (ImGui::BeginMenu("File...")) {

				uint32_t stage = sa::ShaderStageFlagBits::VERTEX;
				while (stage != sa::ShaderStageFlagBits::COMPUTE << 1) {
					std::string typeName = sa::to_string((sa::ShaderStageFlagBits)stage) + " Shader";

					if (ImGui::MenuItem(typeName.c_str())) {
						auto newPath = m_openDirectory / ("New " + typeName + " File");
						newPath.replace_extension(".glsl");
						sa::createGlslFile(newPath, (sa::ShaderStageFlagBits)stage);
						editedFile = newPath;
						editingName = newPath.stem().generic_string();
						wasChanged = true;
					}
					stage = stage << 1;
				}

				ImGui::Separator();
				
				if (ImGui::MenuItem("Lua Script")) {
					auto newPath = m_openDirectory / "New Lua Script";
					newPath.replace_extension(".lua");
					sa::createLuaFile(newPath);
					editedFile = newPath;
					editingName = newPath.stem().generic_string();
					wasChanged = true;
				}

				ImGui::EndMenu();
			}

			ImGui::Separator();
			static std::vector<sa::AssetTypeID> types;
			sa::AssetManager::get().getRegisteredAssetTypes(types);
			for (auto type : types) {
				if (ImGui::GetAssetInfo(type).inCreateMenu) {
					std::string typeName = sa::AssetManager::get().getAssetTypeName(type);
					if (ImGui::MenuItem(typeName.c_str())) {
						sa::Asset* pAsset = sa::AssetManager::get().createAsset(type, "New " + typeName + ".asset", m_openDirectory);
						editingName = pAsset->getName();
						editedFile = pAsset->getAssetPath();
					}
				}
			}
		};

		if (ImGui::BeginDirectoryIcons("Explorer", m_openDirectory, iconSize, wasChanged, editedFile, editingName, lastSelected, selectedItems, menuItemsFn)) {
			SA_PROFILE_SCOPE("Explorer");
			// Icon View Area
			ImVec2 iconSizeVec((float)iconSize, (float)iconSize);
			for (const auto& entry : std::filesystem::directory_iterator(m_openDirectory)) {


				// Determine Icon
				sa::Texture2D icon = m_otherFileIcon;
				if (entry.is_directory()) {
					icon = m_directoryIcon;
				}
				else if(entry.path().extension() == ".lua") {
					icon = m_luaScriptIcon;
				}

				sa::Asset* pAsset = sa::AssetManager::get().findAssetByPath(entry.path());
				if (pAsset) {
					icon = ImGui::GetAssetInfo(pAsset->getType()).icon;
				}

				ImGui::DirectoryEntry(entry, iconSize, selectedItems, lastSelected, wasChanged, editedFile, editingName, icon);

				if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenOverlapped) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
					if (entry.is_directory()) {
						selectedItems.clear();
						lastSelected.clear();
						m_openDirectory = entry.path();
						break;
					}
					sa::Asset* pAsset = sa::AssetManager::get().findAssetByPath(entry.path());
					if (pAsset) {
						if (sa::AssetManager::get().isType<sa::Scene>(pAsset)) {
							m_pEngine->setScene(pAsset->cast<sa::Scene>());
						}
						else {
							pAsset->load();
							m_openAssetProperties.insert(pAsset);
						}
					}
				}

				ImGui::SameLine();

				if (ImGui::GetContentRegionMax().x - ImGui::GetCursorPosX() < iconSizeVec.x) {
					ImGui::NewLine();
				}
			}

			ImGui::EndChild();
		}

		if (wasChanged) {
			sa::AssetManager::get().rescanAssets();
		}

	}
	ImGui::End();

	if (!m_isAssetListOpen)
		return;
	if (ImGui::Begin("Assets", &m_isAssetListOpen)) {
		{
			static std::string path;
			ImGui::InputText("Path", &path);

			if (ImGui::Button("Import")) {
				sa::AssetManager::get().importAsset<sa::ModelAsset>(path);
			}
		}

		ImGui::Separator();

		auto& assets = sa::AssetManager::get().getAssets();
		static sa::Asset* selected = nullptr;

		if (ImGui::Button("Load Asset") && selected) {
			selected->load();
		}
		ImGui::SameLine();
		if (ImGui::Button("Release Asset") && selected) {
			selected->release();
		}
		ImGui::SameLine();
		if (ImGui::Button("Write Asset") && selected) {
			selected->write();
		}

		if (selected && selected->isLoaded()) {
			if (selected->getType() == sa::AssetManager::get().getAssetTypeID<sa::ModelAsset>()) {
				if (ImGui::Button("Spawn")) {
					sa::Entity entity = m_pEngine->getCurrentScene()->createEntity();
					entity.addComponent<comp::Transform>();
					entity.addComponent<comp::Model>()->modelID = selected->getID();
					selected->load();
				}
			}
			else if (selected->getType() == sa::AssetManager::get().getAssetTypeID<sa::Scene>()) {
				if (ImGui::Button("Set Scene")) {
					m_pEngine->setScene(static_cast<sa::Scene*>(selected));
				}
			}
		}

		if (ImGui::BeginChildFrame(ImGui::GetCurrentWindow()->GetID("asset_table"), ImGui::GetContentRegionAvail())) {
			if (ImGui::BeginTable("Asset Table", 5, ImGuiTableFlags_SizingFixedFit)) {
				ImGui::TableSetupColumn("Name");
				ImGui::TableSetupColumn("Type");
				ImGui::TableSetupColumn("Is Loaded");
				ImGui::TableSetupColumn("Asset Path");
				ImGui::TableSetupColumn("References");

				ImGui::TableHeadersRow();

				for (auto& [id, asset] : assets) {
					ImGui::TableNextRow();

					ImGui::TableNextColumn();
					if (ImGui::Selectable(asset->getName().c_str(), selected == asset.get())) {
						selected = asset.get();
					}

					ImGui::TableNextColumn();
					ImGui::TextUnformatted(sa::AssetManager::get().getAssetTypeName(asset->getType()).c_str());

					ImGui::TableNextColumn();
					if (asset->isLoaded()) {
						ImGui::Text("Loaded");
					}
					else {
						ImGui::Text("Unloaded");
					}

					ImGui::TableNextColumn();
					if (!asset->isLoaded() && !asset->getProgress().isAllDone()) {
						ImGui::ProgressBar(asset->getProgress().getAllCompletion());
					}
					else {
						ImGui::TextUnformatted(asset->getAssetPath().generic_string().c_str());
					}

					ImGui::TableNextColumn();
					ImGui::Text("%d", asset->getReferenceCount());

				}
				ImGui::EndTable();
			}
		}
		ImGui::EndChildFrame();
	}
	ImGui::End();

}

void DirectoryView::update(float dt) {

}
