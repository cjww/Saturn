#pragma once

#include "EditorModule.h"
#include "EntityInspector.h"
#include "EditorView.h"

class SceneView : public EditorModule {
private:
	EntityInspector* m_pInspector;
	EditorView* m_pView;

	void makePopups();

public:
	SceneView(sa::Engine* pEngine, EntityInspector* pInspector, EditorView* pView);
	virtual ~SceneView();
	virtual void onImGui() override;
	virtual void update(float dt) override;
};