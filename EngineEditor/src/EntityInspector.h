#pragma once
#include "EditorModule.h"
#include "CustomImGui.h"


class EntityInspector : public EditorModule {

	sa::Entity m_selectedEntity;
	entt::registry* m_pRegistry;

	void makePopups();

public:
	EntityInspector(sa::Engine* pEngine);
	virtual ~EntityInspector();
	virtual void onImGui() override;
	virtual void update(float dt) override;

};