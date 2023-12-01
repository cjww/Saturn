#pragma once

#include "IRenderTechnique.h"
#include "IRenderLayer.h"
#include "Tools/Profiler.h"
#include "RenderLayers\BloomRenderLayer.h"

namespace sa {

	class RenderPipeline {
	private:
		
		std::vector<IRenderLayer*> m_renderLayers;
	public:
		RenderPipeline();
		virtual ~RenderPipeline();

		void addLayer(IRenderLayer* pLayer);

		void onRenderTargetResize(UUID renderTargetID, Extent oldExtent, Extent newExtent);
		
		void beginFrameImGUI();
		
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
