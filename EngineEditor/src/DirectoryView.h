#pragma once
#include "EditorModule.h"
#include "CustomImgui.h"

class DirectoryView : public EditorModule {
private:
	std::list<sa::IAsset*> m_loadingAssets;
public:
	DirectoryView(sa::Engine* pEngine, sa::EngineEditor* pEditor);

	virtual void onImGui() override;
	virtual void update(float dt) override;

};