#include "EntityFactory.h"

EntityFactory::EntityFactory() {

}

EntityID EntityFactory::createEntity() {
	EntityID id;
	if (!m_availableEntities.empty()) {
		id = m_availableEntities.front();
		m_availableEntities.pop();
	}
	id = m_createdEntityCount++;
	return id;
}

void EntityFactory::destroyEntity(EntityID id) {
	m_availableEntities.push(id);
}
