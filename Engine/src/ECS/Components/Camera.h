#pragma once
#include "ECS\ComponentBase.h"

#include "SceneCamera.h"

namespace sa {
	struct RenderTarget;
}
namespace comp {
	class Camera : public sa::ComponentBase {
	public:
		sa::SceneCamera camera;
		sa::RenderTarget* pRenderTarget;

		bool isPrimary = false;

		void* windowCallbackConnection;

		Camera() = default;

		Camera(const Camera&) = default;
		Camera& operator=(const Camera& other) = default;
		Camera& operator=(Camera&&) = default;

		void setRenderTarget(sa::RenderTarget* pRenderTarget);

		virtual void serialize(sa::Serializer& s) override;
		virtual void deserialize(void* pDoc) override;

		virtual void onConstruct(sa::Entity* entity) override;
		virtual void onDestroy(sa::Entity* entity) override;

	};
}