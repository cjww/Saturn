#pragma once
#include "EditorModule.h"


class SceneHierarchy : public EditorModule {
private:
	
	void makePopups();

public:
	
	SceneHierarchy(sa::Engine* pEngine);
	virtual ~SceneHierarchy();
	virtual void onImGui() override;
	virtual void update(float dt) override;
};