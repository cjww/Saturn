#pragma once
#include "Entity.h"

namespace sa {
	class Scene;
	class SceneCamera;
	
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
			Extent oldExtent;
			Extent newExtent;
		};

	}
	namespace scene_event {

		struct SceneUpdate {
			float deltaTime;
		};

		struct SceneStart {};
		struct SceneStop {};

		struct SceneRequest {
			std::string sceneName;
		};

		struct AddedCamera {
			SceneCamera* cam;
		};

		struct RemovedCamera {
			SceneCamera* cam;
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