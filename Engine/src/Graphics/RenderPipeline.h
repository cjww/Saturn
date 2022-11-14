#pragma once

#include "IRenderTechnique.h"
#include "RenderWindow.hpp"
#include "IRenderLayer.h"

namespace sa {

	class RenderPipeline {
	private:
		RenderWindow* m_pWindow;
		IRenderTechnique* m_pRenderTechnique;
		RenderContext m_context;

		std::vector<IRenderLayer*> m_layers;
		std::vector<IRenderLayer*> m_overlays;

		tf::Executor m_executor;

		RenderTarget m_swapchainRenderTarget;
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

		bool render(Scene* pScene);

		RenderContext beginScene(Scene* pScene);
		void render(Camera* pCamera, RenderTarget* rendertarget = nullptr);
		void endScene();

		IRenderTechnique* getRenderTechnique() const;
	};

}
