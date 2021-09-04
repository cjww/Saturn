#include "LightSystem.h"

void LightSystem::update(float dt) {
	for (const auto& e : m_entitySet) {
		Light* light = ECSCoordinator::get()->getComponent<Light>(e);
		glm::vec3 pos = ECSCoordinator::get()->getComponent<Transform>(e)->position;



	}
}
