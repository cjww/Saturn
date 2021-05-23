#pragma once
#include "EntityFactory.h"
#include "ComponentFactory.h"
#include "SystemFactory.h"

#include <set>

// Use this to create entities and add/get/remove components from them
class ECSCoordinator {
private:
	EntityFactory m_entityFactory;
	ComponentFactory m_componentFactory;
	SystemFactory m_systemFactory;

public:
	
	ECSCoordinator();
	// Creates a new entity pointer
	Entity createEntity();
	// Destroys this entity. The entity pointer should be considered invalid after a call to this function
	void destroyEntity(Entity entity);

	// get the type id of a component
	template<typename T>
	ComponentType getComponentType() const;

	// register a new component, has to be called beafore use
	template<typename T>
	void registerComponent();
	
	// registers and returns a new system to make sure there is only one instance of this system
	template<typename T, typename ...Args>
	T* registerSystem(ComponentMask signature, Args&... args);
	
	// Adds a component to an entity
	template<typename T, typename ...Args>
	T* addComponent(Entity entity, Args... args);

	// Gets a component from an entity 
	template<typename T>
	T* getComponent(Entity entity) const;

	// Removes a component from an entity
	template<typename T>
	void removeComponent(Entity entity);

};

template<typename T>
inline ComponentType ECSCoordinator::getComponentType() const {
	return m_componentFactory.getComponentType<T>();
}

template<typename T>
inline void ECSCoordinator::registerComponent() {
	m_componentFactory.registerComponent<T>();
}

template<typename T, typename ...Args>
inline T* ECSCoordinator::registerSystem(ComponentMask signature, Args & ...args) {
	return m_systemFactory.registerSystem<T>(signature, args...);
}

template<typename T, typename ...Args>
inline T* ECSCoordinator::addComponent(Entity entity, Args... args) {
	entity->getSignature().set(m_componentFactory.getComponentType<T>());
	T* comp = m_componentFactory.addComponent<T>(entity->id, args...);
	m_systemFactory.onEntitySignatureChange(entity);
	return comp;
}

template<typename T>
inline T* ECSCoordinator::getComponent(Entity entity) const {
	if (!entity->getSignature().test(getComponentType<T>())) {
		return nullptr;
	}
	return m_componentFactory.getComponent<T>(entity->id);
}

template<typename T>
inline void ECSCoordinator::removeComponent(Entity entity) {
	entity->getSignature().reset(m_componentFactory.getComponentType<T>());
	m_systemFactory.onEntitySignatureChange(entity);
	return m_componentFactory.removeComponent<T>(entity->id);
}

