#pragma once
#include "EditorModule.h"
#include "CustomImgui.h"

class DirectoryView : public EditorModule {
private:
	bool m_isAssetListOpen;
	sa::Texture2D m_directoryIcon;
	sa::Texture2D m_otherFileIcon;

	std::unordered_set<sa::Asset*> m_openAssetProperties;

public:
	DirectoryView(sa::Engine* pEngine, sa::EngineEditor* pEditor);

	virtual void onImGui() override;
	virtual void update(float dt) override;

};