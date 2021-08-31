#include "EntityFactory.h"

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
	setEntityName(id, "Entity");
	return id;
}

void EntityFactory::destroyEntity(EntityID id) {
	m_entitySignatures[id].reset();
	m_availableEntities.push(id);
}

ComponentMask EntityFactory::getEntitySignature(EntityID entity) const {
	return m_entitySignatures.at(entity);
}

void EntityFactory::setEntitySignature(EntityID entity, ComponentMask signature) {
	m_entitySignatures[entity] = signature;
}

const std::string& EntityFactory::getEntityName(EntityID entity) const {
	if (m_entityNames.find(entity) == m_entityNames.end()) {
		return "No Name";
	}
	return m_entityNames.at(entity);
}

void EntityFactory::setEntityName(EntityID entity, const std::string& name) {
	m_entityNames[entity] = name;
}

uint32_t EntityFactory::getEntityCount() const {
	return m_createdEntityCount - m_availableEntities.size();
}

std::vector<EntityID> EntityFactory::getActiveEntities() const {
	std::vector<EntityID> entities;
	entities.reserve(getEntityCount());
	for (EntityID i = 0; i < m_createdEntityCount; i++) {
		if (m_entitySignatures.find(i) != m_entitySignatures.end() && !m_entitySignatures.at(i).none()) {
			entities.push_back(i);
		}
	}
	return std::move(entities);
}
