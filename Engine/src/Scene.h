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
		std::set<Camera*> m_activeCameras;

		static void onModelConstruct(entt::registry reg, entt::entity e);
		static void onModelDestroy(entt::registry reg, entt::entity e);

	public:
		Scene();
		virtual ~Scene();

		void update(float dt);
		void render();

		Camera* newCamera();
		Camera* newCamera(const RenderWindow* pWindow);

		void addActiveCamera(Camera* camera);
		void removeActiveCamera(Camera* camera);
		std::set<Camera*> getActiveCameras() const;

		void setScene(const std::string& name);

		Entity createEntity(const std::string name = "Entity");

		entt::registry& getRegistry();


	};
}
