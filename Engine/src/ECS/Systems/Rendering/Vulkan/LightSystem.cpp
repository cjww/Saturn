#include "LightSystem.h"

void LightSystem::update(float dt) {
	for (const auto& e : m_entitySet) {
		comp::Light* light = ECSCoordinator::get()->getComponent<comp::Light>(e);
		glm::vec3 pos = ECSCoordinator::get()->getComponent<comp::Transform>(e)->position;



	}
}
