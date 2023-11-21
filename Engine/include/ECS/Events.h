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
			inline static const char* CallbackName = "onUpdate";
			float deltaTime;
		};

		struct SceneStart{
			inline static const char* CallbackName = "onStart";
		};
		struct SceneStop {
			inline static const char* CallbackName = "onStop";
		};

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
			inline static const char* CallbackName = "onEntityDestruction";
			Entity entity;
		};

		struct EntityCreated {
			inline static const char* CallbackName = "onEntityCreation";
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