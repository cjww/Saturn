#pragma once

#include "EditorModule.h"

class SceneView : public EditorModule {

public:
	SceneView(sa::Engine* pEngine);
	virtual ~SceneView();
	virtual void onImGui() override;
	virtual void update(float dt) override;
};