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
		RenderContext m_context;

		std::vector<IRenderLayer*> m_layers;
		std::vector<IRenderLayer*> m_overlays;

		tf::Executor m_executor;

		ResourceID m_swapchainFramebuffer;
		ResourceID m_swapchainPipeline;
		ResourceID m_swapchainRenderProgram;
		ResourceID m_swapchainDescriptorSet;
		ResourceID m_sampler;

		
	public:
		RenderPipeline();
		virtual ~RenderPipeline();

		void onWindowResize(Extent newExtent);

		void create(RenderWindow* pWindow, IRenderTechnique* pRenderTechnique);
		
		void pushLayer(IRenderLayer* pLayer);
		void pushOverlay(IRenderLayer* pLayer);

		void beginFrameImGUI();

		RenderContext beginScene(Scene* pScene);
		void render(SceneCamera* pCamera, RenderTarget* pRenderTarget = nullptr);
		void endScene();

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
