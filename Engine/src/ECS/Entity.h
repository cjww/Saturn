#pragma once
#include <entt\entt.hpp>

class Entity {
private:
	entt::registry *m_pRegistry;
	entt::entity m_entity;
public:
	Entity(entt::registry* pRegistry, entt::entity entity);
	~Entity();

	entt::entity getID() const;

	template<typename T>
	T* addComponent();

	template<typename T>
	T* getComponent();

	template<typename T>
	void removeComponent();
};

template<typename T>
inline T* Entity::addComponent() {
	return &m_pRegistry->emplace_or_replace<T>(m_entity);
}

template<typename T>
inline T* Entity::getComponent() {
	return m_pRegistry->try_get<T>(m_entity);
}

template<typename T>
inline void Entity::removeComponent() {
	m_pRegistry->remove<T>(m_entity);
}
