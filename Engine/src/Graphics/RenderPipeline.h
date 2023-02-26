#pragma once

#include "IRenderTechnique.h"
#include "RenderWindow.hpp"
#include "IRenderLayer.h"
#include "Tools/Profiler.h"

namespace sa {

	class RenderPipeline {
	private:
		RenderWindow* m_pWindow;
		IRenderTechnique* m_pRenderTechnique;
		
		std::vector<IRenderLayer*> m_layers;
		std::vector<IRenderLayer*> m_overlays;

		tf::Executor m_executor;
		
	public:
		RenderPipeline();
		virtual ~RenderPipeline();

		void onWindowResize(Extent newExtent);

		void create(RenderWindow* pWindow, IRenderTechnique* pRenderTechnique);
		
		void pushLayer(IRenderLayer* pLayer);
		void pushOverlay(IRenderLayer* pLayer);

		void beginFrameImGUI();

		
		void render(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sceneCollection);
		const Texture& endScene(RenderContext& context);

		IRenderTechnique* getRenderTechnique() const;

		template<typename T>
		T* getLayer() const;

	};

	template<typename T>
	inline T* RenderPipeline::getLayer() const {
		for (auto layer : m_layers) {
			T* l = dynamic_cast<T*>(layer);
			if (l)
				return l;
		}

		for (auto layer : m_overlays) {
			T* l = dynamic_cast<T*>(layer);
			if (l)
				return l;
		}

		return nullptr;
	}


}
