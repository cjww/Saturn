#include "ComponentFactory.h"

ComponentFactory::ComponentFactory() 
	: m_registeredComponentCount(0)
{

}

void ComponentFactory::onEntityDestroyed(EntityID entity, ComponentMask signature) {
	// remove entity from every component type
	for (auto& it : m_componentArrays) {
		if (signature.test(it.first)) {
			it.second->remove(entity);
		}
	}
}
