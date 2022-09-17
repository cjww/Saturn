#pragma once
#include "Entity.h"

namespace sa {
	class Scene;
	class Camera;
	
	namespace engine_event {

		struct SceneSet {
			Scene* newScene;
		};

		struct WindowResized {
			Extent newExtent;
		};

	}
	namespace scene_event {

		struct SceneRequest {
			std::string sceneName;
		};

		struct AddedCamera {
			Camera* cam;
		};

		struct RemovedCamera {
			Camera* cam;
		};

		struct EntityDestroyed {
			Entity entity;
		};

		struct EntityCreated {
			Entity entity;
		};

		struct UpdatedScene {
			float dt;
		};

		template<typename T>
		struct ComponentCreated {
			Entity entity;
			T& component;
		};

		template<typename T>
		struct ComponentDestroyed {
			Entity entity;
			T& component;
		};
	}
}