#include "System.h"

System::System() {
}

void System::removeEntity(EntityID entity) {
	m_entities.erase(entity);
}

void System::addEntity(EntityID entity) {
	m_entities.emplace(entity);
}
