#include "System.h"

System::System() {
}

void System::removeEntity(EntityID entity) {
	m_entitySet.erase(entity);
}

void System::addEntity(EntityID entity) {
	m_entitySet.emplace(entity);
}
