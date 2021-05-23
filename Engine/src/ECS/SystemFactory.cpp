#include "SystemFactory.h"

void SystemFactory::onEntitySignatureChange(Entity entity) {
	for (auto& it : m_systems) {
		ComponentMask systemSignature = m_signatures[it.first];
		
		if ((systemSignature & entity->getSignature()) == systemSignature) {
			// All component requirements are met
			it.second->addEntity(entity);
		}
		else {
			it.second->removeEntity(entity);
		}
	}
}

void SystemFactory::onEntityDestroyed(Entity entity) {
	// inform all systems that this entity was destroyed
	for (auto& it : m_systems) {
		it.second->removeEntity(entity);
	}
}
