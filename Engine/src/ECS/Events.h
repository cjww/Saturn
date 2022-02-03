#pragma once
#include "Camera.h"
#include "Entity.h"

namespace sa {
	namespace event {

		struct SceneSet {
			std::string newSceneName;
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
	}
}