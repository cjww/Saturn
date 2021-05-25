#include "Saturn/ECS/SystemFactory.h"

void SystemFactory::onEntitySignatureAdd(EntityID entity, ComponentMask oldSignature, ComponentMask newSignature) {
	for (auto& it : m_systems) {
		ComponentQuery query = m_queries[it.first];
		if (query.check(newSignature)) {
			// All component requirements are met
			if (!query.check(oldSignature)) {
				// was not added since before
				it.second->addEntity(entity);
			}

		}
	}
}

void SystemFactory::onEntitySignatureRemove(EntityID entity, ComponentMask oldSignature, ComponentMask newSignature) {
	for (auto& it : m_systems) {
		ComponentQuery query = m_queries[it.first];
		if (query.check(oldSignature)) {
			// this system should contain this entity
			if (!query.check(newSignature)) {
				// Now it will not anymore
				it.second->removeEntity(entity);
			}
		}
	}

}


void SystemFactory::onEntityDestroyed(EntityID entity, ComponentMask signature) {
	// inform all systems that this entity was destroyed
	for (auto& it : m_systems) {
		ComponentQuery query = m_queries[it.first];
		if (query.check(signature)) {
			it.second->removeEntity(entity);
		}
	}
}
