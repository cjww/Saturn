#include "ComponentFactory.h"

ComponentFactory::ComponentFactory() 
	: m_registeredComponentCount(0)
{

}

void ComponentFactory::onEntityDestroyed(EntityID entity) {
	// remove entity from every component type
	for (auto& it : m_componentArrays) {
		it.second->remove(entity);
	}
}
