#pragma once
#include "EditorModule.h"


class SceneHierarchy : public EditorModule {
private:
	
	sa::Entity m_selectedEntity;
	sa::Entity m_hoveredEntity;

	bool m_isPopupMenuOpen;

	std::queue<std::pair<sa::Entity, sa::Entity>> m_parentChanges;

	void makePopups();

	void elementEvents(const sa::Entity& e);
	void makeTree(sa::Entity e);


	void onEntitySelected(const sa::editor_event::EntitySelected& e);
	void onEntityDeselected(const sa::editor_event::EntityDeselected& e);
	void onSceneSet(const sa::engine_event::SceneSet& e);


public:
	
	SceneHierarchy(sa::Engine* pEngine, sa::EngineEditor* pEditor);
	virtual ~SceneHierarchy();
	virtual void onImGui() override;
	virtual void update(float dt) override;
};