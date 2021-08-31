#pragma once

#include "EditorModule.h"
#include "EntityInspector.h"

class SceneView : public EditorModule {
private:
	EntityInspector* m_pInspector;
	
	void makePopups();

public:
	SceneView(sa::Engine* pEngine, EntityInspector* pInspector);
	virtual ~SceneView();
	virtual void onImGui() override;
	virtual void update(float dt) override;
};