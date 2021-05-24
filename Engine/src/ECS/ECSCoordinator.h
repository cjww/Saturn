#pragma once
#include "EntityFactory.h"
#include "ComponentFactory.h"
#include "SystemFactory.h"

// Use this to create entities and add/get/remove components from them
class ECSCoordinator {
private:
	EntityFactory m_entityFactory;
	ComponentFactory m_componentFactory;
	SystemFactory m_systemFactory;

public:
	
	ECSCoordinator();
	// Creates a new entity pointer
	EntityID createEntity();
	// Destroys this entity. The entity pointer should be considered invalid after a call to this function
	void destroyEntity(EntityID entity);

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
	template<typename T>
	T* addComponent(EntityID entity);

	// Gets a component from an entity 
	template<typename T>
	T* getComponent(EntityID entity) const;

	// Removes a component from an entity
	template<typename T>
	void removeComponent(EntityID entity);

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

template<typename T>
inline T* ECSCoordinator::addComponent(EntityID entity) {	
	ComponentMask signature = m_entityFactory.getEntitySignature(entity);
	signature.set(m_componentFactory.getComponentType<T>());

	m_systemFactory.onEntitySignatureAdd(entity, m_entityFactory.getEntitySignature(entity), signature);
	m_entityFactory.setEntitySignature(entity, signature);

	T* comp = m_componentFactory.addComponent<T>(entity);
	return comp;
}

template<typename T>
inline T* ECSCoordinator::getComponent(EntityID entity) const {
	ComponentMask signature = m_entityFactory.getEntitySignature(entity);
	if (!signature.test(m_componentFactory.getComponentType<T>())) {
		return nullptr;
	}
	return m_componentFactory.getComponent<T>(entity);
}

template<typename T>
inline void ECSCoordinator::removeComponent(EntityID entity) {
	
	ComponentMask signature = m_entityFactory.getEntitySignature(entity);
	signature.reset(m_componentFactory.getComponentType<T>());
	
	m_systemFactory.onEntitySignatureRemove(entity, m_entityFactory.getEntitySignature(entity), signature);
	m_entityFactory.setEntitySignature(entity, signature);

	m_componentFactory.removeComponent<T>(entity);
}

