#pragma once
#include "EditorModule.h"
#include "CustomImGui.h"


class EntityInspector : public EditorModule {

	sa::Entity m_selectedEntity;
	entt::registry* m_pRegistry;

	void makePopups();

	void onEntitySelected(const sa::editor_event::EntitySelected& e);
	void onEntityDeselected(const sa::editor_event::EntityDeselected& e);


public:
	EntityInspector(sa::Engine* pEngine, sa::EngineEditor* pEditor);
	virtual ~EntityInspector();
	virtual void onImGui() override;
	virtual void update(float dt) override;

};