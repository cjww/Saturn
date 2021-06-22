#include "ECSCoordinator.h"

ECSCoordinator* ECSCoordinator::m_pMyInstance = nullptr;

ECSCoordinator::ECSCoordinator()
{

}

ECSCoordinator* ECSCoordinator::get() {
	if (m_pMyInstance == nullptr) {
		m_pMyInstance = new ECSCoordinator;
	}
	return m_pMyInstance;
}

void ECSCoordinator::cleanup() {
	if (m_pMyInstance != nullptr)
		delete m_pMyInstance;
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

uint32_t ECSCoordinator::getEntityCount() const {
	return m_entityFactory.getEntityCount();
}
