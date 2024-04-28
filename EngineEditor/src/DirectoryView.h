#pragma once
#include "EditorModule.h"


class DirectoryView : public EditorModule {
private:
	bool m_isAssetListOpen;
	sa::Texture m_directoryIcon;
	sa::Texture m_otherFileIcon;
	sa::Texture m_luaScriptIcon;

	std::unordered_set<sa::Asset*> m_openAssetProperties;

	std::filesystem::path m_openDirectory;

	int m_iconSize;
	
	std::filesystem::path m_lastSelected;
	std::set<std::filesystem::path> m_selectedItems;
	
	std::set<std::filesystem::path> m_clipboard;

	std::string m_editingName;
	std::filesystem::path m_editedFile;

	void onDraggedDropped(const sa::editor_event::DragDropped& e);
	void onProjectOpened(const sa::editor_event::ProjectOpened& e);

	void makeAssetPropertiesWindows();
	void makeAssetWindow();

	bool makeDirectoryBackButton(bool& wasChanged);

	bool makePopupContextWindow(bool& wasChanged);

	bool beginDirectoryView(const char* str_id, bool& wasChanged, const ImVec2& size = ImVec2(0, 0));
	void endDirectoryView();
	bool directoryEntry(const std::filesystem::directory_entry& entry, bool& wasChanged, const sa::Texture& icon);


public:
	DirectoryView(sa::Engine* pEngine, sa::EngineEditor* pEditor);

	virtual void onImGui() override;
	virtual void update(float dt) override;


};