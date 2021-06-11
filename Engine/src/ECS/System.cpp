#include "System.h"

System::System() {
}

System::~System() {
}

bool System::removeEntity(EntityID entity) {
	return m_entitySet.erase(entity) > 0;
}

bool System::addEntity(EntityID entity) {
	return m_entitySet.emplace(entity).second;
}
