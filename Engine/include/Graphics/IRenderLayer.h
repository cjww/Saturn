#pragma once

#include "Renderer.hpp"

#include "SceneCamera.h"
#include "SceneCollection.h"
#include "RenderTarget.h"


namespace sa {
	class BasicRenderLayer {
	private:
		bool m_isActive;

		
	protected:
		bool m_isInitialized;
		Renderer& m_renderer;

	public:
		BasicRenderLayer();
		virtual ~BasicRenderLayer() = default;

		virtual void init() = 0;
		virtual void cleanup() = 0;

		virtual void onRenderTargetResize(UUID renderTargetID, Extent oldExtent, Extent newExtent) = 0;

		virtual bool preRender(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sceneCollection) = 0;
		virtual bool render(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sceneCollection) = 0;
		virtual bool postRender(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sceneCollection) = 0;

		bool isActive() const;
		void setActive(bool active);

	};


	template<typename RenderData, typename Preferences>
	class IRenderLayer : public BasicRenderLayer {
	private:
		std::unordered_map<UUID, RenderData> m_renderData;
		Preferences m_preferences;
		
	public:
		IRenderLayer();
		virtual ~IRenderLayer() = default;

		RenderData& getRenderTargetData(const UUID& renderTargetID);
		Preferences& getPreferences();

	};


	template <typename RenderData, typename Preferences>
	IRenderLayer<RenderData, Preferences>::IRenderLayer()
		: BasicRenderLayer()
		, m_preferences({})
	{
	}

	template <typename RenderData, typename Preferences>
	RenderData& IRenderLayer<RenderData, Preferences>::getRenderTargetData(const UUID& renderTargetID) {
		return m_renderData[renderTargetID];
	}

	template <typename RenderData, typename Preferences>
	Preferences& IRenderLayer<RenderData, Preferences>::getPreferences() {
		return m_preferences;
	}


}
