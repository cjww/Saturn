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
		/*
		std::string extension = path.extension().generic_string();
		if (sa::ModelAsset::IsExtensionSupported(extension)) {
			sa::AssetManager::Get().importAsset<sa::ModelAsset>(path, m_openDirectory);
			continue;
		}
		auto pTextureAsset = sa::AssetManager::Get().importAsset<sa::TextureAsset>(path, m_openDirectory);
		if(pTextureAsset)
			continue;

		SA_DEBUG_LOG_WARNING("Could not import: Unsupported extension ", extension);
		*/
	}
}

void DirectoryView::onProjectOpened(const sa::editor_event::ProjectOpened& e) {
	assert(!std::filesystem::is_directory(e.projectPath));
	setOpenDirectory(e.projectPath.parent_path() / SA_ASSET_DIR);
}

void DirectoryView::updateDirectoryEntries() {
	m_currentFileEntries.clear();
	for (const auto& entry : std::filesystem::directory_iterator(m_openDirectory)) {
		if (entry.path().extension() == SA_META_ASSET_EXTENSION) {
			continue; // Hide meta files
		}
		addFileEntry(std::filesystem::proximate(entry.path()));
	}
}

DirectoryView::FileEntry& DirectoryView::addFileEntry(const std::filesystem::path& path) {
	if (sa::Asset* pAsset = sa::AssetManager::Get().findAssetByPath(path)) {
		return addFileEntry(pAsset);
	}
	// Determine Icon
	FileEntry file = {};
	file.path = path;
	file.icon = m_otherFileIcon;
	if (std::filesystem::is_directory(path)) {
		file.isDirectory = true;
		file.icon = m_directoryIcon;
	}
	else if (path.extension() == ".lua") {
		file.icon = m_luaScriptIcon;
	}
	return m_currentFileEntries.emplace_back(file);
}

DirectoryView::FileEntry& DirectoryView::addFileEntry(const sa::Asset* pAsset) {
	FileEntry file = {};
	file.path = pAsset->getAssetPath();
	file.assetType = pAsset->getType();
	file.assetID = pAsset->getID();
	file.icon = ImGui::GetAssetInfo(pAsset->getType()).icon;

	return m_currentFileEntries.emplace_back(file);
}

auto DirectoryView::removeFileEntry(const std::filesystem::path& path) {
	auto it = std::find_if(m_currentFileEntries.begin(), m_currentFileEntries.end(), [&](const FileEntry& entry) { return entry.path == path; });
	if (it != m_currentFileEntries.end()) {
		return m_currentFileEntries.erase(it);
	}
	return m_currentFileEntries.end();
}

void DirectoryView::copyItems(const FileEntrySet& items) {
	m_clipboard.clear();
	m_clipboard.insert(m_selectedItems.begin(), m_selectedItems.end());
}

uint32_t DirectoryView::pasteItems(const std::filesystem::path& targetDirectory) {
	uint32_t itemsSuccess = 0;
	for (auto& entry : m_clipboard) {
		try {
			if (entry.assetType != -1) {
				sa::Asset* pAsset = sa::AssetManager::Get().getAsset(entry.assetID);
				if (pasteAsset(pAsset, targetDirectory)) {
					itemsSuccess++;
				}
			}
			else {
				auto newPath = std::filesystem::proximate(targetDirectory / entry.path.filename());
				std::string stem = newPath.stem().generic_string();
				if (std::filesystem::equivalent(targetDirectory, entry.path.parent_path())) {
					int i = 1;
					while (std::filesystem::exists(newPath)) {
						newPath.replace_filename(stem + " - Copy (" + std::to_string(i) + ")" + newPath.extension().generic_string());
						++i;
					}
				}
				else {
					if (std::filesystem::exists(newPath)) {
						auto msg = newPath.wstring() + L" already exists.\nOverwrite with current file?";
						if (!sa::FileDialogs::YesNoWindow(L"File already exists", msg.c_str(), false)) {
							continue;
						}
					}
				}
				std::filesystem::copy(entry.path, newPath, std::filesystem::copy_options::overwrite_existing);
				if (entry.isDirectory) {
					updateDirectoryEntries();
					sa::AssetManager::Get().rescanAssets();
				}
				SA_DEBUG_LOG_INFO("Pasted ", entry.path, " to ", targetDirectory);
				itemsSuccess++;
			}

		}
		catch (const std::filesystem::filesystem_error& e) {
			SA_DEBUG_LOG_ERROR(e.what(), ": ", e.path1(), " ", e.path2());
		}
	}
	updateDirectoryEntries();
	return itemsSuccess;
}

uint32_t DirectoryView::deleteItems(const FileEntrySet& items) {
	uint32_t success = 0;
	for (auto& entry : items) {
		if (entry.assetType != -1) {
			sa::AssetManager::Get().deleteAsset(entry.assetID);
			success++;
			SA_DEBUG_LOG_INFO("Deleted Asset ", entry.path);
			continue;
		}

		try {
			if (std::filesystem::is_directory(entry.path)) {
				if (!std::filesystem::is_empty(entry.path)) {
					auto msg = L"The directory " + entry.path.wstring() + L" is not empty.\nRemove directory and all contents?";
					if(!sa::FileDialogs::YesNoWindow(L"Non empty directory", msg.c_str())) {
						continue;
					}
				}
				std::filesystem::remove_all(entry.path);
			}
			else {
				std::filesystem::remove(entry.path);
			}
			success++;
			if (entry.isDirectory) {
				updateDirectoryEntries();
				sa::AssetManager::Get().rescanAssets();
			}

			SA_DEBUG_LOG_INFO("Deleted ", entry.path);
		}
		catch (const std::filesystem::filesystem_error& e) {
			SA_DEBUG_LOG_ERROR(e.what(), ": ", e.path1(), " ", e.path2());
		}
	}
	updateDirectoryEntries();
	return success;
}

bool DirectoryView::moveItem(const FileEntry& item, const std::filesystem::path& targetDirectory) {
	bool wasChanged = false;
	try {
		auto newPath = targetDirectory / item.path.filename();
		if (std::filesystem::exists(newPath)) {
			auto msg = newPath.wstring() + L" already exists.\nOverwrite with current file?";
			if (!sa::FileDialogs::YesNoWindow(L"File already exists", msg.c_str(), false)) {
				return false;
			}
		}
		std::filesystem::rename(item.path, newPath);
		wasChanged = true;

		if (item.assetType != -1) {
			sa::Asset* pAsset = sa::AssetManager::Get().getAsset(item.assetID);
			wasChanged = moveAsset(pAsset, targetDirectory);
		}
		
		if (item.isDirectory) {
			updateDirectoryEntries();
			sa::AssetManager::Get().rescanAssets();
		}

		SA_DEBUG_LOG_INFO("Moved ", item.path.filename(), " to ", targetDirectory);
	}
	catch (const std::filesystem::filesystem_error& e) {
		SA_DEBUG_LOG_ERROR(e.what(), ": ", e.path1(), " ", e.path2());
	}
	updateDirectoryEntries();
	return wasChanged;
}

uint32_t DirectoryView::moveItems(const FileEntrySet& items, const std::filesystem::path& targetDirectory) {
	uint32_t success = 0;
	for (auto& file : items) {
		if (moveItem(file, targetDirectory))
			success++;
	}
	return success;
}

bool DirectoryView::renameItem(const FileEntry& item, const std::filesystem::path& name) {
	bool wasChanged = false;
	try {
		std::filesystem::path newName = item.path;
		newName.replace_filename(name);
		newName.replace_extension(item.path.extension());

		std::filesystem::rename(item.path, newName);
		wasChanged = true;
		SA_DEBUG_LOG_INFO("Renamed ", std::filesystem::proximate(item.path), " to ", std::filesystem::proximate(name));

		if (item.assetType != -1) {
			sa::Asset* pAsset = sa::AssetManager::Get().getAsset(item.assetID);
			wasChanged = renameAsset(pAsset, name);
		}
		else if (item.isDirectory) {
			updateDirectoryEntries();
			sa::AssetManager::Get().rescanAssets();
		}

	}
	catch (const std::filesystem::filesystem_error& e) {
		SA_DEBUG_LOG_ERROR(e.what(), ": ", e.path1(), " ", e.path2());
	}
	return wasChanged;
}

bool DirectoryView::renameAsset(sa::Asset* pAsset, const std::filesystem::path& name) const {
	if (!pAsset->isCompiled()) {
		auto metaPath = pAsset->getMetaFilePath();
		auto newPath = metaPath;
		newPath.replace_filename(name);
		newPath.replace_extension(SA_META_ASSET_EXTENSION);
		try {
			std::filesystem::rename(metaPath, newPath);
		}
		catch (const std::filesystem::filesystem_error& e) {
			SA_DEBUG_LOG_ERROR("Failed to rename meta file ", metaPath, ": ", e.what());
			return false;
		}
	}
	std::filesystem::path newPath = pAsset->getAssetPath();
	const std::filesystem::path& extension = newPath.extension();
	newPath.replace_filename(name).replace_extension(extension);
	pAsset->setAssetPath(newPath);
	return true;
}

bool DirectoryView::pasteAsset(sa::Asset* pAsset, const std::filesystem::path& targetDirectory) {
	
	std::string newName = pAsset->getName() + " (Clone)";
	sa::Asset* pClone = pAsset->clone(newName, targetDirectory);
	if (!pClone) {
		SA_DEBUG_LOG_ERROR("Asset ", pAsset->getName(), " could not be cloned");
		return false;
	}

	int i = 1;
	while (std::filesystem::exists(pClone->getAssetPath())) {
		std::filesystem::path newPath = pClone->getAssetPath();
		const std::filesystem::path& extension = newPath.extension();
		newPath.replace_filename(pAsset->getName() + " (Clone " + std::to_string(i) + ")").replace_extension(extension);
		pClone->setAssetPath(newPath);
		++i;
	}
	pClone->write();
	pClone->getProgress().wait();
	return true;

	if (!pAsset->isCompiled()) {
		auto metaPath = pAsset->getMetaFilePath();
		auto newPath  = targetDirectory / metaPath.filename();
		try {
			std::filesystem::copy(metaPath, newPath);
		}
		catch (const std::filesystem::filesystem_error& e) {
			SA_DEBUG_LOG_ERROR("Failed to copy meta file ", metaPath, " to ", newPath, ": ", e.what());
			return false;
		}
	}
	std::filesystem::path assetPath = pAsset->getAssetPath();
	assetPath = targetDirectory / assetPath.filename();
	pAsset->setAssetPath(assetPath);
	return true;
}

bool DirectoryView::moveAsset(sa::Asset* pAsset, const std::filesystem::path& targetDirectory) {
	if (!pAsset->isCompiled()) {
		try {
			const auto& metaPath = pAsset->getMetaFilePath();
			auto newPath = targetDirectory / metaPath.filename();
			std::filesystem::rename(metaPath, newPath);
			SA_DEBUG_LOG_INFO("Moved ", metaPath, " to ", targetDirectory);
		}
		catch (const std::filesystem::filesystem_error& e) {
			SA_DEBUG_LOG_ERROR(e.what(), ": ", e.path1(), " ", e.path2());
			return false;
		}
	}

	std::filesystem::path assetPath = pAsset->getAssetPath();
	assetPath = targetDirectory / assetPath.filename();
	pAsset->setAssetPath(assetPath);
	return true;
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
		ImGui::SameLine();
		if (ImGui::Button("Clone") && selected) {
			selected->clone(selected->getName() + " - Clone");
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


bool DirectoryView::makeDirectoryBackButton() {

	if (ImGui::ArrowButton("parentDir_button", ImGuiDir_Left)) {
		if (m_openDirectory.has_parent_path()) {
			setOpenDirectory(m_openDirectory.parent_path());
			m_selectedItems.clear();
			m_lastSelected = {};
			return true;
		}
	}
	return false;
}

bool DirectoryView::makeDirectoryDragDropTarget(const std::filesystem::path& path) {
	bool movedItem = false;
	if (ImGui::BeginDragDropTarget()) {
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Path");
		if (payload && payload->IsDelivery()) {
			FileEntrySet* selectedItems = static_cast<FileEntrySet*>(payload->Data);
			if (moveItems(*selectedItems, std::filesystem::proximate(path)) > 0) {
				selectedItems->clear();
				movedItem = true;
			}
		}
		ImGui::EndDragDropTarget();
	}
	return movedItem;
}


bool DirectoryView::beginDirectoryView(const char* str_id, const ImVec2& size) {
	ImVec2 contentArea = size;
	if (size.x == 0.f && size.y == 0.f) {
		contentArea = ImGui::GetContentRegionAvail();
	}

	bool began = ImGui::BeginChild((std::string(str_id) + "1").c_str(), contentArea, false, ImGuiWindowFlags_MenuBar);
	if (!began) {
		return false;
	}

	//Menu Bar
	if (ImGui::BeginMenuBar()) {

		if (!std::filesystem::equivalent(m_openDirectory, SA_ASSET_DIR)) {
			makeDirectoryBackButton();
			makeDirectoryDragDropTarget(m_openDirectory.parent_path());
		
		}

		ImGui::Text(std::filesystem::absolute(m_openDirectory).generic_string().c_str());
		
		float size = ImGui::GetCurrentWindow()->MenuBarHeight();
		if (ImGui::ImageButtonTinted(m_refreshButton, ImVec2(size, size))) {
			updateDirectoryEntries();
			sa::AssetManager::Get().rescanAssets();
		}

		ImGui::SetNextItemWidth(200);
		ImGui::SliderInt("Icon size", &m_iconSize, 20, 200);
		
	}
	ImGui::EndMenuBar();	
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
	m_refreshButton.create2D(
		sa::Image(m_pEditor->MakeEditorRelative("resources/refresh_button-white.png").generic_string()),
		false);



	m_openDirectory = std::filesystem::current_path();

	m_iconSize = 45;
}

bool DirectoryView::makeContextMenu() {
	bool inContextMenu = ImGui::BeginPopupContextWindow();
	//Input Events
	if (inContextMenu) {
		
		if (ImGui::BeginMenu("Create...")) {
			if (ImGui::MenuItem("Folder")) {
				auto newPath = m_openDirectory / "New Folder";
				if (std::filesystem::create_directory(m_openDirectory / "New Folder")) {
					m_editingName = "New Folder";
					m_editedFile = addFileEntry(newPath);
				}
			}

			if (ImGui::BeginMenu("File...")) {

				uint32_t stage = sa::ShaderStageFlagBits::VERTEX;
				while (stage != sa::ShaderStageFlagBits::COMPUTE << 1) {
					std::string typeName = sa::to_string((sa::ShaderStageFlagBits)stage) + " Shader";

					if (ImGui::MenuItem(typeName.c_str())) {
						auto newPath = m_openDirectory / ("New " + typeName + " File");
						newPath.replace_extension(".glsl");
						sa::createGlslFile(newPath, (sa::ShaderStageFlagBits)stage);
						m_editingName = newPath.stem().generic_string();
						m_editedFile = addFileEntry(newPath);
					}
					stage = stage << 1;
				}

				ImGui::Separator();

				if (ImGui::MenuItem("Lua Script")) {
					auto newPath = m_openDirectory / "New Lua Script";
					newPath.replace_extension(".lua");
					sa::createLuaFile(newPath);
					m_editingName = newPath.stem().generic_string();
					m_editedFile = addFileEntry(newPath);
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
						m_editedFile = addFileEntry(pAsset);
					}
				}
			}

			ImGui::EndMenu();
		}

		if (m_lastSelected) {
			if (ImGui::MenuItem("Rename", "F2")) {
				m_editedFile = m_lastSelected;
				m_editingName = m_lastSelected.path.stem().generic_string();
			}
		}

		if (!m_clipboard.empty()) {
			if (ImGui::MenuItem("Paste", "Ctrl + V")) {
				if (pasteItems(m_openDirectory) > 0) {
					m_selectedItems.clear();
					m_lastSelected = {};
				}
			}
		}
		if (!m_selectedItems.empty()) {
			if (ImGui::MenuItem("Copy", "Ctrl + C")) {
				copyItems(m_selectedItems);
			}

			if (ImGui::MenuItem("Delete", "Del")) {
				if (deleteItems(m_selectedItems) > 0)  {
					m_selectedItems.clear();
					m_lastSelected = {};
				}
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
	return inContextMenu;
}

bool DirectoryView::makeContextMenuShortcuts() {
	if (ImGui::IsKeyPressed(ImGuiKey_F2) && m_lastSelected) {
		m_editedFile = m_lastSelected;
		m_editingName = m_lastSelected.path.stem().generic_string();
	}

	if (ImGui::IsKeyPressed(ImGuiKey_Delete)) {
		if (deleteItems(m_selectedItems) > 0) {
			m_selectedItems.clear();
			m_lastSelected = {};
		}
	}

	if (ImGui::IsKeyPressed(ImGuiKey_C) && ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
		copyItems(m_selectedItems);
	}

	if (ImGui::IsKeyPressed(ImGuiKey_V) && ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
		if (!m_clipboard.empty()) {
			if (pasteItems(m_openDirectory) > 0) {
				m_selectedItems.clear();
				m_lastSelected = {};
			}
		}
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

		
		// Icon View Area
		if (beginDirectoryView("Explorer")) {
			SA_PROFILE_SCOPE("Explorer");
			
			bool inContextMenu = makeContextMenu();
			makeContextMenuShortcuts();


			ImVec2 iconSizeVec((float)m_iconSize, (float)m_iconSize);
			
			ImRect selectionRect = {};
			bool isSelecting = makeMouseDragSelection(selectionRect.Min, selectionRect.Max);
			bool isAItemHovered = false;
			for (auto& file : m_currentFileEntries) {

				bool isBeingDragged = false;
				directoryEntry(file, isBeingDragged);
				file.boundsMin = glm::vec2(ImGui::GetItemRectMin().x, ImGui::GetItemRectMin().y);
				file.boundsMax = glm::vec2(ImGui::GetItemRectMax().x, ImGui::GetItemRectMax().y);
				if (isBeingDragged)
					isSelecting = false;
				
				if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenOverlapped)) {
					isAItemHovered = true;
					if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
						if (file.isDirectory) {
							m_selectedItems.clear();
							m_lastSelected = {};
							setOpenDirectory(file.path);
							break;
						}
						if (file.assetType != -1) {
							sa::Asset* pAsset = sa::AssetManager::Get().getAsset(file.assetID);
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
					}
				}

				ImGui::SameLine();

				if (ImGui::GetContentRegionMax().x - ImGui::GetCursorPosX() < iconSizeVec.x) {
					ImGui::NewLine();
				}
			}

			if (!inContextMenu && !isAItemHovered) {
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
					if (!ImGui::IsKeyDown(ImGuiKey_LeftShift) && !ImGui::IsKeyDown(ImGuiKey_RightShift)) {
						m_selectedItems.clear();
						m_lastSelected = {};
					}
				}
			}

			if (isSelecting) {
				m_lastSelected = {};
				for (const auto& file : m_currentFileEntries) {
					//if intersecting with selection rect
					ImRect itemRect = ImRect(file.boundsMin.x, file.boundsMin.y, file.boundsMax.x, file.boundsMax.y);
					if (selectionRect.Overlaps(itemRect)) {
						m_selectedItems.insert(file);
						m_lastSelected = file;
					}
					else {
						m_selectedItems.erase(file);
					}
				}
				renderSelectionRect(selectionRect.Min, selectionRect.Max);
			}

			endDirectoryView();
		}
	}
	ImGui::End();

}

bool DirectoryView::directoryEntry(const FileEntry& file, bool& isBeingDragged) {
	// Check if selected
	bool selected = m_selectedItems.count(file);
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

	std::string label = file.path.filename().generic_string();
	ImVec2 totalSize = iconSizeVec + ImGui::GetStyle().ItemSpacing * 2.f;
	ImVec2 textSize = ImGui::CalcTextSize(label.c_str(), 0, false, totalSize.x);
	totalSize.y += textSize.y;

	ImGui::BeginGroup();

	ImVec2 cursorPos = ImGui::GetCursorPos();
	if (ImGui::Selectable(("##directory_entry" + label).c_str(), selected, ImGuiSelectableFlags_NoPadWithHalfSpacing, totalSize)) {
		if (!ImGui::IsKeyDown(ImGuiKey_LeftShift) && !ImGui::IsKeyDown(ImGuiKey_RightShift))
			m_selectedItems.clear();
		m_selectedItems.insert(file);
		m_lastSelected = file;
	}
	// Drag drop source
	if (ImGui::BeginDragDropSource()) {
		isBeingDragged = true;
		if (!m_selectedItems.contains(file)) {
			m_selectedItems.clear();
			m_selectedItems.insert(file);
			m_lastSelected = file;
		}
		ImGui::SetDragDropPayload("Path", &m_selectedItems, sizeof(m_selectedItems));
		ImGui::Image(m_lastSelected.icon, iconSizeVec);
		for (auto& entry : m_selectedItems) {
			ImGui::SameLine();
			ImGui::Text("%s", entry.path.filename().generic_string().c_str());
		}
		ImGui::EndDragDropSource();
	}
	// And possibly target
	if (file.isDirectory) {
		makeDirectoryDragDropTarget(file.path);
	}


	cursorPos.x += ImGui::GetStyle().ItemSpacing.x;
	ImGui::SetCursorPos(cursorPos);
	// Icon
	ImGui::Image(file.icon, iconSizeVec);

	// Text field
	cursorPos.x = ImGui::GetCursorPosX();
	if (m_editedFile == file) { 
		// is renaming label
		cursorPos.x += ImGui::GetStyle().ItemInnerSpacing.x;
		ImGui::SetCursorPosX(cursorPos.x);
		ImGui::SetNextItemWidth(totalSize.x - ImGui::GetStyle().ItemInnerSpacing.x * 2.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1, 1));
		if (ImGui::InputText("##edit_name", &m_editingName, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
			if (renameItem(m_editedFile, m_editingName)) {
				m_selectedItems.clear();
				m_lastSelected = {};
				updateDirectoryEntries();
			}
			m_editedFile = {};
		}
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsItemHovered()) {
			m_editedFile = {};
		}
		else {
			ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
		}
		ImGui::PopStyleVar();
	}
	else { 
		// regular label
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

bool DirectoryView::makeMouseDragSelection(ImVec2& outMin, ImVec2& outMax) {
	bool isDragging = ImGui::IsMouseDragging(ImGuiMouseButton_Left);
	if (isDragging) {
		if (!m_wasMouseDown) {
			if (!ImGui::IsWindowHovered()) {
				return false;
			}
			m_wasMouseDown = true;
			m_mousePosStart = ImGui::GetMousePos();
		}
		ImVec2 currentPos = ImGui::GetMousePos();

		outMin = ImVec2(std::min(currentPos.x, m_mousePosStart.x), std::min(currentPos.y, m_mousePosStart.y));
		outMax = ImVec2(std::max(currentPos.x, m_mousePosStart.x), std::max(currentPos.y, m_mousePosStart.y));

	}
	else {
		m_wasMouseDown = false;
	}
	return isDragging;
}

void DirectoryView::renderSelectionRect(const ImVec2& min, const ImVec2& max) {
	ImColor color = ImGui::GetStyleColorVec4(ImGuiCol_NavHighlight);
	ImGui::GetWindowDrawList()->AddRect(min, max, color);
	color.Value.w = 0.5f;
	ImGui::GetWindowDrawList()->AddRectFilled(min, max, color);
}

void DirectoryView::update(float dt) {

}

void DirectoryView::setOpenDirectory(const std::filesystem::path& directory) {
	m_openDirectory = directory;
	updateDirectoryEntries();
}
