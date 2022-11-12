#pragma once

#include "IRenderTechnique.h"
#include "RenderWindow.hpp"
#include "IRenderLayer.h"

namespace sa {


	struct RenderTarget {
		Camera* pCamera;
		ResourceID framebuffer;
		Texture2D colorTexture;
	};

	class RenderPipeline {
	private:
		RenderWindow* m_pWindow;
		IRenderTechnique* m_pRenderTechnique;
		RenderContext m_context;

		std::vector<IRenderLayer*> m_layers;
		std::vector<IRenderLayer*> m_overlays;

		tf::Executor m_executor;

		ResourceID m_swapchainRenderProgram;
		ResourceID m_swapchainPipeline;
		ResourceID m_swapchainDescriptorSet;
		RenderTarget m_swapchainRenderTarget;
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

		bool beginScene(Scene* pScene);
		void render(Camera* pCamera);
		void endScene();

		IRenderTechnique* getRenderTechnique() const;
	};

}
