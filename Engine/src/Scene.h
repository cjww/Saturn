#pragma once
#include "entt\entt.hpp"
#include "Camera.h"

namespace sa {

	class Scene : public entt::emitter<Scene> {
	private:
		entt::registry m_registry;

		std::list<std::unique_ptr<Camera>> m_cameras;
		std::set<Camera*> m_activeCameras;


		void onModelConstruct(entt::registry& r, entt::entity entity);
		void onModelDestroy(entt::registry& r, entt::entity entity);

	public:
		Scene();
		entt::registry& getRegistry();
		Camera* createCamera(glm::vec2 extent);
		void addActiveCamera(Camera* camera);
		void removeActiveCamera(Camera* camera);
		const std::set<Camera*>& getActiveCameras() const;

		void update(float dt);

	};

}
