#include "System.h"

System::System() {
}

bool System::removeEntity(Entity entity) {
	for (int i = 0; i < m_entities.size(); i++) {
		if (m_entities[i] == entity) {
			m_entities.erase(m_entities.begin() + i);
			return true;
		}
	}
	return false;
}

bool System::addEntity(Entity entity) {
	for (int i = 0; i < m_entities.size(); i++) {
		if (m_entities[i] == entity) {
			return false;
		}
	}
	m_entities.push_back(entity);
	return true;
}
