#include "ComponentFactory.h"

ComponentFactory::ComponentFactory() 
	: m_registeredComponentCount(0)
{

}


std::vector<ComponentType> ComponentFactory::getAllComponentTypes() const {
	std::vector<ComponentType> types;
	types.reserve(m_componentTypes.size());
	for (const auto& pair : m_componentTypes) {
		types.push_back(pair.second);
	}
	return std::move(types);
}

const char* ComponentFactory::getComponentName(ComponentType type) const {
	for (const auto& pair : m_componentTypes) {
		if (pair.second == type) {
			return pair.first;
		}
	}
	return "Unrecognized Component";
}

void ComponentFactory::addComponent(ComponentType type, EntityID entity) {
	m_componentArrays.at(type)->insertOnly(entity);
}

void ComponentFactory::removeComponent(ComponentType type, EntityID entity) {
	m_componentArrays.at(type)->remove(entity);
}

void ComponentFactory::onEntityDestroyed(EntityID entity, ComponentMask signature) {
	// remove entity from every component type
	for (auto& it : m_componentArrays) {
		if (signature.test(it.first)) {
			it.second->remove(entity);
		}
	}
}
