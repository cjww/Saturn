#pragma once
#include "EditorModule.h"
#include "CustomImGui.h"


class EntityInspector : public EditorModule {

	EntityID m_currentEntity;

	ComponentType m_removeComponent;

	void makePopups();

public:
	EntityInspector(sa::Engine* pEngine);
	virtual ~EntityInspector();
	virtual void onImGui();
	virtual void update(float dt);

	EntityID getEntity() const;
	void setEntity(EntityID id);


};