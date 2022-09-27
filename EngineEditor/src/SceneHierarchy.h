#pragma once
#include "EditorModule.h"


class SceneHierarchy : public EditorModule {
private:
	
	sa::Entity m_selectedEntity;
	sa::Entity m_hoveredEntity;

	void makePopups();

	void tree(const sa::Entity& child);
public:
	
	SceneHierarchy(sa::Engine* pEngine);
	virtual ~SceneHierarchy();
	virtual void onImGui() override;
	virtual void update(float dt) override;
};