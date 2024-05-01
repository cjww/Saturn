#pragma once
#include "EditorModule.h"


class DirectoryView : public EditorModule {
private:
	bool m_isAssetListOpen;
	sa::Texture m_directoryIcon;
	sa::Texture m_otherFileIcon;
	sa::Texture m_luaScriptIcon;
	sa::Texture m_refreshButton;

	std::unordered_set<sa::Asset*> m_openAssetProperties;

	std::filesystem::path m_openDirectory;

	int m_iconSize;
	


	struct FileEntry {
		std::filesystem::path path;
		bool isDirectory = false;
		sa::Texture icon;
		sa::UUID assetID = 0;
		sa::AssetTypeID assetType = ~0u;

		glm::vec2 boundsMin;
		glm::vec2 boundsMax;

		bool operator<(const FileEntry& other) const { return path < other.path; }
		bool operator==(const FileEntry& other) const { return path == other.path; }

		operator bool() const { return !path.empty(); };
	};
	
	FileEntry m_lastSelected;
	
	std::string m_editingName;
	FileEntry m_editedFile;

	std::vector<FileEntry> m_currentFileEntries;

	typedef std::set<FileEntry> FileEntrySet;

	FileEntrySet m_selectedItems;
	FileEntrySet m_clipboard;
	
	ImVec2 m_mousePosStart;
	bool m_wasMouseDown;

	void onDraggedDropped(const sa::editor_event::DragDropped& e);
	void onProjectOpened(const sa::editor_event::ProjectOpened& e);

	void updateDirectoryEntries();

	void makeAssetPropertiesWindows();
	void makeAssetWindow();

	bool makeDirectoryBackButton();
	bool makeDirectoryDragDropTarget(const std::filesystem::path& path);

	bool makeContextMenu();
	bool makeContextMenuShortcuts();


	bool beginDirectoryView(const char* str_id, const ImVec2& size = ImVec2(0, 0));
	void endDirectoryView();
	bool directoryEntry(const FileEntry& file, bool& isBeingDragged);

	bool makeMouseDragSelection(ImVec2& outMin, ImVec2& outMax);
	void renderSelectionRect(const ImVec2& min, const ImVec2& max);

	FileEntry& addFileEntry(const std::filesystem::path& path);
	FileEntry& addFileEntry(const sa::Asset* pAsset);
	auto removeFileEntry(const std::filesystem::path& path);

	void copyItems(const FileEntrySet& items);
	uint32_t pasteItems(const std::filesystem::path& targetDirectory);
	uint32_t deleteItems(const FileEntrySet& items);
	bool moveItem(const FileEntry& item, const std::filesystem::path& targetDirectory);
	uint32_t moveItems(const FileEntrySet& items, const std::filesystem::path& targetDirectory);
	bool renameItem(const FileEntry& item, const std::filesystem::path& name);

	bool renameAsset(sa::Asset* pAsset, const std::filesystem::path& name) const;
	bool pasteAsset(sa::Asset* pAsset, const std::filesystem::path& targetDirectory);
	bool moveAsset(sa::Asset* pAsset, const std::filesystem::path& targetDirectory);


public:
	DirectoryView(sa::Engine* pEngine, sa::EngineEditor* pEditor);

	virtual void onImGui() override;
	virtual void update(float dt) override;


	void setOpenDirectory(const std::filesystem::path& directory);

};

