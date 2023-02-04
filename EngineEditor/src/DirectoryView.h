#pragma once
#include "EditorModule.h"
#include "CustomImgui.h"

class DirectoryView : public EditorModule {

public:
	DirectoryView(sa::Engine* pEngine, sa::EngineEditor* pEditor);

	virtual void onImGui() override;
	virtual void update(float dt) override;

};