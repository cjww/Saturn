#pragma once
#include "ECS\ComponentBase.h"

#include "SceneCamera.h"
#include "Graphics\RenderTarget.h"
#include "Graphics\SceneCollection.h"
#include "Assets/AssetHolder.h"

namespace comp {
	class Camera : public sa::ComponentBase {
	private:
		sa::AssetHolder<sa::RenderTarget> m_renderTarget;

		void onRendertargetResize(const sa::engine_event::RenderTargetResized& e);

	public:
		sa::SceneCamera camera;
		sa::SceneCollection sceneCollection;

		Camera();
		Camera(const Camera&);
		Camera(Camera&&);
		Camera& operator=(const Camera& other);
		Camera& operator=(Camera&&);


		virtual void serialize(sa::Serializer& s) override;
		virtual void deserialize(void* pDoc) override;

		virtual void onConstruct(sa::Entity* entity) override;
		virtual void onDestroy(sa::Entity* entity) override;

		const sa::AssetHolder<sa::RenderTarget>& getRenderTarget() const;
		void setRenderTarget(sa::RenderTarget* pRenderTarget);

		
	};
}