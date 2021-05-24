#include "ECSCoordinator.h"

ECSCoordinator::ECSCoordinator()
{

}

EntityID ECSCoordinator::createEntity() {
	return m_entityFactory.createEntity();
}

void ECSCoordinator::destroyEntity(EntityID entity) {
	m_componentFactory.onEntityDestroyed(entity);
	m_systemFactory.onEntityDestroyed(entity);
	m_entityFactory.destroyEntity(entity);
}
