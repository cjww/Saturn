#include "DirectoryView.h"

#include "AssetManager.h"
#include "Assets/ModelAsset.h"

#include "EngineEditor.h"

#include "FileTemplates.h"

#include "Tools\FileDialogs.h"

#include "imgui_internal.h"

#include "CustomImgui.h"

void DirectoryView::onDraggedDropped(const sa::editor_event::DragDropped& e) {
	for (uint32_t i = 0; i < e.count; i++) {
		std::filesystem::path path = e.paths[i];
		std::string extension = path.extension().generic_string();
		if (sa::ModelAsset::IsExtensionSupported(extension)) {
			sa::AssetManager::Get().importAsset<sa::ModelAsset>(path, m_openDirectory);
			continue;
		}
		auto pTextureAsset = sa::AssetManager::Get().importAsset<sa::TextureAsset>(path, m_openDirectory);
		if(pTextureAsset)
			continue;

		SA_DEBUG_LOG_WARNING("Could not import: Unsupported extension ", extension);
	}
}

void DirectoryView::onProjectOpened(const sa::editor_event::ProjectOpened& e) {
	m_openDirectory = std::filesystem::is_directory(e.projectPath)? e.projectPath : e.projectPath.parent_path();
}

void DirectoryView::makeAssetPropertiesWindows() {
	for (auto it = m_openAssetProperties.begin(); it != m_openAssetProperties.end(); ) {
		sa::Asset* pAsset = *it;
		SA_PROFILE_SCOPE(sa::AssetManager::Get().getAssetTypeName(pAsset->getType()), " Properties Window");
		bool isOpen = true;
		if (ImGui::Begin((pAsset->getName() + " Properties").c_str(), &isOpen)) {
			if (!isOpen) {
				it = m_openAssetProperties.erase(it);
				pAsset->release();
				ImGui::End();
				continue;
			}
			if (pAsset->isLoaded()) {
				ImGui::GetAssetInfo(pAsset->getType()).imGuiPropertiesFn(pAsset);
				if (ImGui::Button("Apply")) {
					pAsset->write();
				}
				ImGui::SameLine();
				if (ImGui::Button("Revert")) {
					pAsset->load();
				}
				if (sa::AssetManager::Get().wasImported(pAsset)) {
					ImGui::SameLine();
					if (ImGui::Button("Reimport")) {
						sa::AssetManager::Get().reimportAsset(pAsset);
					}
				}
			}
		}
		ImGui::End();
		it++;
	}
}

void DirectoryView::makeAssetWindow() {
	if (!m_isAssetListOpen)
		return;
	if (ImGui::Begin("Assets", &m_isAssetListOpen)) {
		{
			static std::string path;
			ImGui::InputText("Path", &path);

			if (ImGui::Button("Import")) {
				sa::AssetManager::Get().importAsset<sa::ModelAsset>(path);
			}
		}

		ImGui::Separator();

		auto& assets = sa::AssetManager::Get().getAssets();
		static sa::Asset* selected = nullptr;

		if (ImGui::Button("Hold Asset") && selected) {
			selected->hold();
		}
		ImGui::SameLine();
		if (ImGui::Button("Release Asset") && selected) {
			selected->release();
		}
		ImGui::SameLine();
		if (ImGui::Button("Write Asset") && selected) {
			selected->write();
		}
		ImGui::SameLine();
		if (ImGui::Button("Properties") && selected) {
			if (m_openAssetProperties.insert(selected).second)
				selected->hold();
		}


		if (selected && selected->isLoaded()) {
			if (selected->getType() == sa::AssetManager::Get().getAssetTypeID<sa::ModelAsset>()) {
				if (ImGui::Button("Spawn")) {
					sa::Entity entity = m_pEngine->getCurrentScene()->createEntity();
					entity.addComponent<comp::Transform>();
					entity.addComponent<comp::Model>()->model = selected->getID();
				}
			}
			else if (selected->getType() == sa::AssetManager::Get().getAssetTypeID<sa::Scene>()) {
				if (ImGui::Button("Set Scene")) {
					m_pEngine->setScene(static_cast<sa::Scene*>(selected));
				}
			}
		}

		if (ImGui::BeginChildFrame(ImGui::GetCurrentWindow()->GetID("asset_table"), ImGui::GetContentRegionAvail())) {
			if (ImGui::BeginTable("Asset Table", 6, ImGuiTableFlags_SizingFixedFit)) {
				ImGui::TableSetupColumn("Name");
				ImGui::TableSetupColumn("Type");
				ImGui::TableSetupColumn("Is Loaded");
				ImGui::TableSetupColumn("Asset Path");
				ImGui::TableSetupColumn("References");
				ImGui::TableSetupColumn("Size on disk");

				ImGui::TableHeadersRow();

				for (auto& [id, asset] : assets) {
					ImGui::TableNextRow();

					ImGui::TableNextColumn();
					if (ImGui::Selectable(asset->getName().c_str(), selected == asset.get())) {
						selected = asset.get();
					}

					ImGui::TableNextColumn();
					ImGui::TextUnformatted(sa::AssetManager::Get().getAssetTypeName(asset->getType()).c_str());

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
					ImGui::Text("%u", asset->getReferenceCount());

					ImGui::TableNextColumn();
					ImGui::Text("%llu bytes", asset->getHeader().size);


				}
				ImGui::EndTable();
			}
		}
		ImGui::EndChildFrame();
	}
	ImGui::End();
}


bool DirectoryView::makeDirectoryBackButton(bool& wasChanged) {
	if (std::filesystem::equivalent(m_openDirectory, SA_ASSET_DIR)) {
		return false;
	}
	if (ImGui::ArrowButton("parentDir_button", ImGuiDir_Left)) {
		if (m_openDirectory.has_parent_path()) {
			m_openDirectory = m_openDirectory.parent_path();
		}
	}

	if (ImGui::BeginDragDropTarget()) {
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Path");
		if (payload && payload->IsDelivery()) {
			auto path = static_cast<std::filesystem::path*>(payload->Data);
			if (ImGui::MoveItem(*path, m_openDirectory.parent_path())) {
				wasChanged = true;
			}

		}
		ImGui::EndDragDropTarget();
	}

	return true;
}

bool DirectoryView::beginDirectoryView(const char* str_id, bool& wasChanged, const ImVec2& size) {
	ImVec2 contentArea = size;
	if (size.x == 0.f && size.y == 0.f) {
		contentArea = ImGui::GetContentRegionAvail();
	}

	bool began = ImGui::BeginChild((std::string(str_id) + "1").c_str(), contentArea, false, ImGuiWindowFlags_MenuBar);
	if (!began) {
		return false;
	}

	static std::set<std::filesystem::path> copiedFiles;
	//Menu Bar
	if (ImGui::BeginMenuBar()) {

		if(makeDirectoryBackButton(wasChanged)) {
			ImGui::SameLine();
		}

		ImGui::Text(m_openDirectory.generic_string().c_str());
		ImGui::SameLine();

		ImGui::SetNextItemWidth(200);
		ImGui::SliderInt("Icon size", &m_iconSize, 20, 200);
	}
	ImGui::EndMenuBar();

	makePopupContextWindow(wasChanged);
	/*
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
		if (!ImGui::IsKeyDown(ImGuiKey_LeftShift) && !ImGui::IsKeyDown(ImGuiKey_RightShift))
			m_selectedItems.clear();
	}
	*/

	if (ImGui::IsKeyPressed(ImGuiKey_F2) && !m_lastSelected.empty()) {
		m_editedFile = m_lastSelected;
		m_editingName = m_lastSelected.filename().generic_string();
	}

	if (ImGui::IsKeyPressed(ImGuiKey_Delete)) {
		if (ImGui::DeleteItems(m_selectedItems))
			wasChanged = true;
		m_lastSelected.clear();
	}

	if (ImGui::IsKeyPressed(ImGuiKey_C) && ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
		copiedFiles.clear();
		copiedFiles.insert(m_selectedItems.begin(), m_selectedItems.end());
	}

	if (ImGui::IsKeyPressed(ImGuiKey_V) && ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
		if (!copiedFiles.empty()) {
			if (ImGui::PasteItems(copiedFiles, m_openDirectory))
				wasChanged = true;
		}
	}
	
	return true;
}

void DirectoryView::endDirectoryView() {
	ImGui::EndChild();
}

DirectoryView::DirectoryView(sa::Engine* pEngine, sa::EngineEditor* pEditor)
	: EditorModule(pEngine, pEditor, "Directory View", true)
{
	m_isOpen = false;
	m_isAssetListOpen = false;

	m_pEngine->sink<sa::editor_event::DragDropped>().connect<&DirectoryView::onDraggedDropped>(this);
	m_pEngine->sink<sa::editor_event::ProjectOpened>().connect<&DirectoryView::onProjectOpened>(this);


	m_directoryIcon.create2D(
		sa::Image(m_pEditor->MakeEditorRelative("resources/folder-white.png").generic_string()),
		true);
	m_otherFileIcon.create2D(
		sa::Image(m_pEditor->MakeEditorRelative("resources/file-white.png").generic_string()),
		true);
	m_luaScriptIcon.create2D(
		sa::Image(m_pEditor->MakeEditorRelative("resources/lua_file-white.png").generic_string()),
		true);


	m_openDirectory = std::filesystem::current_path();

	m_iconSize = 45;
}

bool DirectoryView::makePopupContextWindow(bool& wasChanged) {
	//Input Events
	if (ImGui::BeginPopupContextWindow()) {

		if (ImGui::BeginMenu("Create...")) {
			if (ImGui::MenuItem("Folder")) {
				auto newPath = m_openDirectory / "New Folder";
				std::filesystem::create_directory(m_openDirectory / "New Folder");
				m_editedFile = newPath;
				m_editingName = "New Folder";
				wasChanged = true;
			}

			if (ImGui::BeginMenu("File...")) {

				uint32_t stage = sa::ShaderStageFlagBits::VERTEX;
				while (stage != sa::ShaderStageFlagBits::COMPUTE << 1) {
					std::string typeName = sa::to_string((sa::ShaderStageFlagBits)stage) + " Shader";

					if (ImGui::MenuItem(typeName.c_str())) {
						auto newPath = m_openDirectory / ("New " + typeName + " File");
						newPath.replace_extension(".glsl");
						sa::createGlslFile(newPath, (sa::ShaderStageFlagBits)stage);
						m_editedFile = newPath;
						m_editingName = newPath.stem().generic_string();
						wasChanged = true;
					}
					stage = stage << 1;
				}

				ImGui::Separator();

				if (ImGui::MenuItem("Lua Script")) {
					auto newPath = m_openDirectory / "New Lua Script";
					newPath.replace_extension(".lua");
					sa::createLuaFile(newPath);
					m_editedFile = newPath;
					m_editingName = newPath.stem().generic_string();
					wasChanged = true;
				}

				ImGui::EndMenu();
			}

			ImGui::Separator();
			static std::vector<sa::AssetTypeID> types;
			sa::AssetManager::Get().getRegisteredAssetTypes(types);
			for (auto type : types) {
				if (ImGui::GetAssetInfo(type).inCreateMenu) {
					std::string typeName = sa::AssetManager::Get().getAssetTypeName(type);
					if (ImGui::MenuItem(typeName.c_str())) {
						sa::Asset* pAsset = sa::AssetManager::Get().createAsset(type, "New " + typeName, m_openDirectory);
						pAsset->write();
						m_editingName = pAsset->getName();
						m_editedFile = pAsset->getAssetPath();
					}
				}
			}

			ImGui::EndMenu();
		}

		if (ImGui::MenuItem("Rename", "F2")) {
			m_editedFile = m_lastSelected;
			m_editingName = m_lastSelected.filename().generic_string();
		}

		if (!m_clipboard.empty()) {
			if (ImGui::MenuItem("Paste", "Ctrl + V")) {
				if (ImGui::PasteItems(m_clipboard, m_openDirectory))
					wasChanged = true;
			}
		}
		if (!m_selectedItems.empty()) {
			if (ImGui::MenuItem("Copy", "Ctrl + C")) {
				m_clipboard.clear();
				m_clipboard.insert(m_selectedItems.begin(), m_selectedItems.end());
			}

			if (ImGui::MenuItem("Delete", "Del")) {
				if (ImGui::DeleteItems(m_selectedItems))
					wasChanged = true;
				m_lastSelected.clear();
			}
		}

		ImGui::Separator();

		if (ImGui::MenuItem("Open in code editor")) {
			//TODO Get code editor executable from editor settings
			if (!sa::FileDialogs::OpenFileInTextEditor("F:/Microsoft VS Code/Code.exe", m_openDirectory)) {
				SA_DEBUG_LOG_ERROR("Failed to open in code editor");
			}
		}

		ImGui::EndPopup();
	}
	return true;
}

void DirectoryView::onImGui() {
	SA_PROFILE_FUNCTION();
	
	makeAssetPropertiesWindows();

	makeAssetWindow();

	if (!m_isOpen)
		return;
	
	if (ImGui::Begin(m_name, &m_isOpen, ImGuiWindowFlags_MenuBar)) {
		
		if (ImGui::BeginMenuBar()) {
			if (ImGui::MenuItem("Assets Window")) {
				m_isAssetListOpen = !m_isAssetListOpen;
			}
			ImGui::EndMenuBar();
		}


		bool wasChanged = false;

		if (beginDirectoryView("Explorer", wasChanged)) {
			SA_PROFILE_SCOPE("Explorer");
			// Icon View Area
			ImVec2 iconSizeVec((float)m_iconSize, (float)m_iconSize);
			for (const auto& entry : std::filesystem::directory_iterator(m_openDirectory)) {


				// Determine Icon
				sa::Texture icon = m_otherFileIcon;
				if (entry.is_directory()) {
					icon = m_directoryIcon;
				}
				else if(entry.path().extension() == ".lua") {
					icon = m_luaScriptIcon;
				}
				
				sa::Asset* pAsset = nullptr;
				sa::AssetTypeID assetType = sa::AssetManager::Get().getAssetTypeByFile(entry.path());
				if(sa::AssetManager::IsCompiledAsset(entry) || assetType != -1) {
					pAsset = sa::AssetManager::Get().findAssetByPath(entry.path());
					if (pAsset) {
						icon = ImGui::GetAssetInfo(pAsset->getType()).icon;
					}
				}

				directoryEntry(entry, wasChanged, icon);
				
				if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenOverlapped) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
					if (entry.is_directory()) {
						m_selectedItems.clear();
						m_lastSelected.clear();
						m_openDirectory = entry.path();
						break;
					}
					if (pAsset) {
						if (sa::AssetManager::Get().isType<sa::Scene>(pAsset)) {
							m_pEngine->setScene(pAsset->cast<sa::Scene>());
						}
						else {
							if (m_openAssetProperties.insert(pAsset).second)
								pAsset->hold();
						}
					}
				}

				ImGui::SameLine();

				if (ImGui::GetContentRegionMax().x - ImGui::GetCursorPosX() < iconSizeVec.x) {
					ImGui::NewLine();
				}
			}

			endDirectoryView();
		}

		if (wasChanged) {
			sa::AssetManager::Get().rescanAssets();
		}

	}
	ImGui::End();

}

bool DirectoryView::directoryEntry(const std::filesystem::directory_entry& entry, bool& wasChanged, const sa::Texture& icon) {
	// Check if selected
	bool selected = m_selectedItems.count(entry.path());
	ImVec2 iconSizeVec = ImVec2(m_iconSize, m_iconSize);
	// Selected highlight color
	int popCount = 0;
	if (selected) {
		ImVec4 selectedCol = ImGui::GetStyleColorVec4(ImGuiCol_CheckMark);
		ImVec4 hoveredCol = selectedCol + ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered) * ImVec4(0.3f, 0.3f, 0.3f, 0.3f);
		ImGui::PushStyleColor(ImGuiCol_Header, selectedCol);
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, hoveredCol);
		popCount = 2;
	}

	// Draw icon and label

	std::string label = entry.path().filename().generic_string();
	ImVec2 totalSize = iconSizeVec + ImGui::GetStyle().ItemSpacing * 2.f;
	ImVec2 textSize = ImGui::CalcTextSize(label.c_str(), 0, false, totalSize.x);
	totalSize.y += textSize.y;

	ImGui::BeginGroup();

	ImVec2 cursorPos = ImGui::GetCursorPos();
	if (ImGui::Selectable(("##directory_entry" + label).c_str(), selected, ImGuiSelectableFlags_NoPadWithHalfSpacing, totalSize)) {
		if (!ImGui::IsKeyDown(ImGuiKey_LeftShift) && !ImGui::IsKeyDown(ImGuiKey_RightShift))
			m_selectedItems.clear();
		m_selectedItems.insert(entry.path());
		m_lastSelected = entry.path();
	}
	// Drag drop source
	if (ImGui::BeginDragDropSource()) {
		ImGui::SetDragDropPayload("Path", &m_selectedItems, sizeof(m_selectedItems));
		ImGui::Image(icon, iconSizeVec);
		ImGui::SameLine();
		for (auto& path : m_selectedItems) {
			ImGui::Text("%s", path.filename().generic_string().c_str());
		}
		ImGui::EndDragDropSource();
	}
	// And possibly target
	if (entry.is_directory()) {
		if (ImGui::BeginDragDropTarget()) {
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Path");
			if (payload && payload->IsDelivery()) {
				std::set<std::filesystem::path>* selectedItems = static_cast<std::set<std::filesystem::path>*>(payload->Data);
				const auto& thisPath = entry.path();
				for (auto& path : *selectedItems) {
					if (ImGui::MoveItem(path, thisPath))
						wasChanged = true;
				}
			}
			ImGui::EndDragDropTarget();
		}
	}


	cursorPos.x += ImGui::GetStyle().ItemSpacing.x;
	ImGui::SetCursorPos(cursorPos);
	// Icon
	ImGui::Image(icon, iconSizeVec);

	// Text field
	cursorPos.x = ImGui::GetCursorPosX();
	if (m_editedFile == entry.path()) {
		cursorPos.x += ImGui::GetStyle().ItemInnerSpacing.x;
		ImGui::SetCursorPosX(cursorPos.x);
		ImGui::SetNextItemWidth(totalSize.x - ImGui::GetStyle().ItemInnerSpacing.x * 2.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
		if (ImGui::InputText("##edit_name", &m_editingName, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
			//rename
			std::filesystem::path newName = (m_editedFile.parent_path() / m_editingName).replace_extension(m_editedFile.extension());
			if (ImGui::RenameItem(m_editedFile, newName))
				wasChanged = true;
			m_editedFile.clear();
		}
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsItemHovered()) {
			m_editedFile.clear();
		}
		else {
			ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
		}
		ImGui::PopStyleVar();
	}
	else {
		cursorPos.x += (totalSize.x - textSize.x) * 0.5f;
		ImGui::SetCursorPosX(cursorPos.x);
		ImGui::PushTextWrapPos(cursorPos.x + textSize.x);
		ImGui::TextWrapped("%s", label.c_str());
		ImGui::PopTextWrapPos();
	}

	ImGui::EndGroup();

	ImGui::PopStyleColor(popCount);

	return false;
}

void DirectoryView::update(float dt) {

}
