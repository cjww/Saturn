#pragma once
#include "EditorModule.h"
#include "CustomImgui.h"

class DirectoryView : public EditorModule {
private:
	bool m_isAssetListOpen;
	sa::Texture2D m_directoryIcon;
	sa::Texture2D m_otherFileIcon;
	sa::Texture2D m_luaScriptIcon;

	std::unordered_set<sa::Asset*> m_openAssetProperties;

	void onDraggedDropped(const sa::editor_event::DragDropped& e);

public:
	DirectoryView(sa::Engine* pEngine, sa::EngineEditor* pEditor);

	virtual void onImGui() override;
	virtual void update(float dt) override;


};