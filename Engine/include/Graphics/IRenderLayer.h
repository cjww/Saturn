#pragma once

#include "Renderer.hpp"

#include "SceneCamera.h"
#include "SceneCollection.h"
#include "RenderTarget.h"


namespace sa {
	class IRenderLayer {
	private:
		bool m_isActive;
	protected:
		Renderer& m_renderer;

		bool m_isInitialized;

		virtual void* getData(const UUID& renderTargetID) = 0;
	public:
		IRenderLayer();
		virtual ~IRenderLayer() = default;

		virtual void init() = 0;
		virtual void cleanup() = 0;

		virtual void onRenderTargetResize(UUID renderTargetID, Extent oldExtent, Extent newExtent) = 0;

		virtual bool preRender(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sceneCollection) = 0;
		virtual bool render(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sceneCollection) = 0;
		virtual bool postRender(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sceneCollection) = 0;

		template<typename T>
		T* getRenderTargetData(const UUID& renderTargetID);

		bool isActive() const;
		void setActive(bool active);

	};

	template <typename T>
	inline T* IRenderLayer::getRenderTargetData(const UUID& renderTargetID) {
		return static_cast<T*>(getData(renderTargetID));
	}
}
