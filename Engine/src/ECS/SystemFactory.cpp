#include "SystemFactory.h"

void SystemFactory::onEntitySignatureChange(EntityID entity, ComponentMask newSignature) {
	for (auto& it : m_systems) {
		ComponentMask systemSignature = m_signatures[it.first];

		if ((systemSignature & newSignature) == systemSignature) {
			// All component requirements are met
			it.second->addEntity(entity);
		}
		else {
			it.second->removeEntity(entity);
		}
	}

}

void SystemFactory::onEntitySignatureAdd(EntityID entity, ComponentMask oldSignature, ComponentMask newSignature) {
	for (auto& it : m_systems) {
		ComponentMask systemSignature = m_signatures[it.first];

		if ((systemSignature & newSignature) == systemSignature) {
			// All component requirements are met
			if ((systemSignature & oldSignature) != systemSignature) {
				// was not added since before
				it.second->addEntity(entity);
			}

		}
	}
}

void SystemFactory::onEntitySignatureRemove(EntityID entity, ComponentMask oldSignature, ComponentMask newSignature) {
	for (auto& it : m_systems) {
		ComponentMask systemSignature = m_signatures[it.first];
		if ((systemSignature & oldSignature) == systemSignature) {
			// this system should contain this entity
			if ((systemSignature & newSignature) != systemSignature) {
				// Now it will not anymore
				it.second->removeEntity(entity);
			}
		}
	}

}


void SystemFactory::onEntityDestroyed(EntityID entity) {
	// inform all systems that this entity was destroyed
	for (auto& it : m_systems) {
		it.second->removeEntity(entity);
	}
}
