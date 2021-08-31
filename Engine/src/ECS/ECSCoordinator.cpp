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

EntityID ECSCoordinator::createEntity(const std::string& name) {
	EntityID e = createEntity();
	setEntityName(e, name);
	return e;
}

void ECSCoordinator::destroyEntity(EntityID entity) {
	ComponentMask signature = m_entityFactory.getEntitySignature(entity);

	m_componentFactory.onEntityDestroyed(entity, signature);
	m_systemFactory.onEntityDestroyed(entity, m_entityFactory.getEntitySignature(entity));
	m_entityFactory.destroyEntity(entity);
}

const std::string& ECSCoordinator::getEntityName(EntityID entity) const {
	return m_entityFactory.getEntityName(entity);
}

void ECSCoordinator::setEntityName(EntityID entity, const std::string& name) {
	m_entityFactory.setEntityName(entity, name);
}

uint32_t ECSCoordinator::getEntityCount() const {
	return m_entityFactory.getEntityCount();
}

std::vector<EntityID> ECSCoordinator::getActiveEntities() const {
	return std::move(m_entityFactory.getActiveEntities());
}

std::vector<ComponentType> ECSCoordinator::getAllComponentTypes() const {
	return std::move(m_componentFactory.getAllComponentTypes());
}

const char* ECSCoordinator::getComponentName(ComponentType type) const {
	return m_componentFactory.getComponentName(type);
}

void ECSCoordinator::addComponent(ComponentType type, EntityID entity) {
	ComponentMask signature = m_entityFactory.getEntitySignature(entity);
	if (signature.test(type)) {
		return; // already added
	}
	signature.set(type);

	m_systemFactory.onEntitySignatureAdd(entity, m_entityFactory.getEntitySignature(entity), signature);
	m_entityFactory.setEntitySignature(entity, signature);

	m_componentFactory.addComponent(type, entity);
}

void ECSCoordinator::removeComponent(ComponentType type, EntityID entity) {

	ComponentMask signature = m_entityFactory.getEntitySignature(entity);
	if (!signature.test(type)) {
		return; // does not contain this component
	}
	signature.reset(type);

	m_systemFactory.onEntitySignatureRemove(entity, m_entityFactory.getEntitySignature(entity), signature);
	m_entityFactory.setEntitySignature(entity, signature);

	m_componentFactory.removeComponent(type, entity);

}