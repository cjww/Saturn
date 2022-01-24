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
	virtual void onImGui();
	virtual void update(float dt);

	sa::Entity getEntity() const;
	void setEntity(const sa::Entity& entity);


};