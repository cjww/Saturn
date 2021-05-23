#include "ECSCoordinator.h"

ECSCoordinator::ECSCoordinator()
{

}

Entity ECSCoordinator::createEntity() {
	EntityID id = m_entityFactory.createEntity();
	return std::make_shared<EntityWrapper>(EntityWrapper(id));
}

void ECSCoordinator::destroyEntity(Entity entity) {
	m_componentFactory.onEntityDestroyed(entity->id);
	m_systemFactory.onEntityDestroyed(entity);
	m_entityFactory.destroyEntity(entity->id);
}
