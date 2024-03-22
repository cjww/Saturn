#pragma once

#include "IRenderTechnique.h"
#include "IRenderLayer.h"
#include "Tools/Profiler.h"

namespace sa {

	class RenderPipeline {
	private:
		std::vector<BasicRenderLayer*> m_renderLayers;
		
	public:
		RenderPipeline();
		virtual ~RenderPipeline();

		void addLayer(BasicRenderLayer* pLayer);

		void onRenderTargetResize(UUID renderTargetID, Extent oldExtent, Extent newExtent);
		void updatePreferences();

		void beginFrameImGUI();
		
		void preRender(RenderContext& context, SceneCollection& sceneCollection);
		void render(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sceneCollection);

		template<typename T>
		T* getLayer() const;
	};

	template <typename T>
	T* RenderPipeline::getLayer() const {
		for(auto& layer : m_renderLayers) {
			T* ptr = dynamic_cast<T*>(layer);
			if (ptr)
				return ptr;
		}
		return nullptr;
	}
}
