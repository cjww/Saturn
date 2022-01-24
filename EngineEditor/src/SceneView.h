#pragma once

#include "EditorModule.h"
#include "EntityInspector.h"
#include "EditorView.h"


class SceneView : public EditorModule {
private:
	
	void makePopups();

public:
	
	SceneView(sa::Engine* pEngine);
	virtual ~SceneView();
	virtual void onImGui() override;
	virtual void update(float dt) override;
};