#pragma once
#include "ComponentArray.h"
#include <unordered_map>
#include <memory>
#include <bitset>

typedef uint32_t ComponentType;

class ComponentFactory {
private:
	// Stores all unique Component IDs and maps to their names
	std::unordered_map<const char*, ComponentType> m_componentTypes;
	// Maps each Component ID to an array containing all Components (MAX_ENTITY_COUNT) of this type
	std::unordered_map<ComponentType, std::shared_ptr<ComponentArrayBase>> m_componentArrays;

	// Used to provide a new unique ID to newly registered Component
	uint32_t m_registeredComponentCount;

	template<typename T>
	std::shared_ptr<ComponentArray<T>> getComponentArray() const;

public:

	ComponentFactory();

	template<typename T>
	ComponentType getComponentType() const;

	// Gives the new Component a unique ID and creates an array containing MAX_ENTITY_COUNT of this new Component
	template<typename T>
	void registerComponent();
	
	// Asks ComponentArray of this ComponentType to map this entity ID to a Component in the array
	template<typename T>
	T* addComponent(EntityID entity);
	
	// Asks ComponentArray of this ComponentType to return mapped Component data
	template<typename T>
	T* getComponent(EntityID entity) const;

	// Asks ComponentArray of this ComponentType to unmap this entity to this ComponentType
	template<typename T>
	void removeComponent(EntityID entity);

	// Makes sure all ComponentArrays unmaps this Entity
	void onEntityDestroyed(EntityID entity, ComponentMask signature);

};



template<typename T>
inline std::shared_ptr<ComponentArray<T>> ComponentFactory::getComponentArray() const {
	ComponentType type = getComponentType<T>();
	return std::static_pointer_cast<ComponentArray<T>>(m_componentArrays.at(type));
}

template<typename T>
inline ComponentType ComponentFactory::getComponentType() const {
	const char* name = typeid(T).name();
	if (m_componentTypes.find(name) == m_componentTypes.end()) {
		throw std::runtime_error("Component not registered");
	}
	return m_componentTypes.at(name);
}

template<typename T>
inline void ComponentFactory::registerComponent() {
	const char* name = typeid(T).name();
	if (m_componentTypes.find(name) != m_componentTypes.end()) {
		throw std::runtime_error("Component already registered");
	}

	ComponentType componentId = m_registeredComponentCount++;
	m_componentTypes[name] = componentId;

	m_componentArrays[componentId] = std::make_shared<ComponentArray<T>>();
}

template<typename T>
inline T* ComponentFactory::addComponent(EntityID entity) {
	return getComponentArray<T>()->insert(entity);
}

template<typename T>
inline T* ComponentFactory::getComponent(EntityID entity) const {
	return getComponentArray<T>()->get(entity);
}

template<typename T>
inline void ComponentFactory::removeComponent(EntityID entity) {
	getComponentArray<T>()->remove(entity);
}
