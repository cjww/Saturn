#pragma once
#include "EditorModule.h"
#include "CustomImgui.h"

class DirectoryView : public EditorModule {
private:
	bool m_isAssetListOpen;
	sa::Texture m_directoryIcon;
	sa::Texture m_otherFileIcon;
	sa::Texture m_luaScriptIcon;

	std::unordered_set<sa::Asset*> m_openAssetProperties;

	std::filesystem::path m_openDirectory;

	void onDraggedDropped(const sa::editor_event::DragDropped& e);
	void onProjectOpened(const sa::editor_event::ProjectOpened& e);

public:
	DirectoryView(sa::Engine* pEngine, sa::EngineEditor* pEditor);

	virtual void onImGui() override;
	virtual void update(float dt) override;


};