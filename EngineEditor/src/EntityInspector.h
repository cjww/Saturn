#pragma once
#include "EditorModule.h"
#include "CustomImGui.h"



template<typename T>
T* get(entt::registry& r, entt::entity e) {
	return r.try_get<T>(e);
}

template<typename T>
T* add(entt::registry& r, entt::entity e) {
	return &r.emplace<T>(e);
}


template<typename T>
void remove(entt::registry& r, entt::entity e) {
	r.remove<T>(e);
}



class EntityInspector : public EditorModule {

	entt::entity m_currentEntity;
	entt::registry* m_pRegistry;

	ComponentType m_removeComponent;

	void makePopups();

public:
	EntityInspector(sa::Engine* pEngine);
	virtual ~EntityInspector();
	virtual void onImGui();
	virtual void update(float dt);

	entt::entity getEntity() const;
	void setEntity(entt::entity id);


};