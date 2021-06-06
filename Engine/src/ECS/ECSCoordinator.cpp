#include "ECSCoordinator.h"

ECSCoordinator::ECSCoordinator()
{

}

ECSCoordinator* ECSCoordinator::get() {
	static ECSCoordinator instance;
	return &instance;
}

EntityID ECSCoordinator::createEntity() {
	return m_entityFactory.createEntity();
}

void ECSCoordinator::destroyEntity(EntityID entity) {
	ComponentMask signature = m_entityFactory.getEntitySignature(entity);

	m_componentFactory.onEntityDestroyed(entity, signature);
	m_systemFactory.onEntityDestroyed(entity, m_entityFactory.getEntitySignature(entity));
	m_entityFactory.destroyEntity(entity);
}
