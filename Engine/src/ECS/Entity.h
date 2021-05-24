#pragma once
#include "ECSCoordinator.h"

class Entity {
private:
	ECSCoordinator* m_pCoordinator;
	EntityID m_id;
public:
	Entity(ECSCoordinator* pCoordinator);
	~Entity();

	EntityID getID() const;

	template<typename T>
	T* addComponent();

	template<typename T>
	T* getComponent();

	template<typename T>
	void removeComponent();
};

template<typename T>
inline T* Entity::addComponent() {
	return m_pCoordinator->addComponent<T>(m_id);
}

template<typename T>
inline T* Entity::getComponent() {
	return m_pCoordinator->getComponent<T>(m_id);
}

template<typename T>
inline void Entity::removeComponent() {
	m_pCoordinator->removeComponent<T>(m_id);
}
