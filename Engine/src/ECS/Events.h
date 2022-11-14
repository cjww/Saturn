#pragma once
#include "Entity.h"

namespace sa {
	class Scene;
	class Camera;
	
	class RenderPipeline;
	class RenderContext;

	namespace engine_event {

		struct OnRender {
			RenderContext* pContext;
			RenderPipeline* pRenderPipeline;
		};

		struct SceneSet {
			Scene* oldScene;
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