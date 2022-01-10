#pragma once
#include "common.hpp"
#include "entt/entt.hpp"
#include "Camera.h"
#include "ECS/Entity.h"

namespace sa {
	typedef uint32_t SceneID;

	// Events
	struct SceneSet {
		std::string newSceneName;
	};

	struct AddCamera {
		Camera* cam;
	};

	struct RemoveCamera {
		Camera* cam;
	};

	class Scene : public entt::emitter<Scene> {
	private:
		entt::registry m_reg;

		std::vector<Camera*> m_cameras;

	public:
		Scene() = default;
		virtual ~Scene();

		Camera* newCamera();
		Camera* newCamera(const RenderWindow* pWindow);

		void addActiveCamera(Camera* camera);
		void removeActiveCamera(Camera* camera);

		void setScene(const std::string& name);

		Entity createEntity(const std::string name = "Entity");

		entt::registry& getRegistry();


	};
}
