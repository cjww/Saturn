#include "Saturn/ECS/EntityFactory.h"

EntityFactory::EntityFactory() {

}

EntityID EntityFactory::createEntity() {
	EntityID id;
	if (!m_availableEntities.empty()) {
		id = m_availableEntities.front();
		m_availableEntities.pop();
	}
	else {
		id = m_createdEntityCount++;
	}
	m_entitySignatures[id] = ComponentMask();
	return id;
}

void EntityFactory::destroyEntity(EntityID id) {
	m_availableEntities.push(id);
}

ComponentMask EntityFactory::getEntitySignature(EntityID entity) const {
	return m_entitySignatures.at(entity);
}

void EntityFactory::setEntitySignature(EntityID entity, ComponentMask signature) {
	m_entitySignatures[entity] = signature;
}
