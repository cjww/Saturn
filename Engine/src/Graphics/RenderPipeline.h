#pragma once

#include "IRenderTechnique.h"
namespace sa {

	class RenderPipeline {
	private:
		RenderWindow* m_pWindow;
		IRenderTechnique* m_pRenderTechnique;
		RenderContext m_context;

		std::vector<IRenderLayer*> m_layers;

		tf::Taskflow m_taskflow;
		tf::Executor m_executor;
	public:
		RenderPipeline();
		virtual ~RenderPipeline();

		void onWindowResize(Extent newExtent);

		void create(RenderWindow* pWindow, IRenderTechnique* pRenderTechnique);

		void pushLayer(IRenderLayer* pLayer);

		void beginFrameImGUI();

		bool render(Scene* pScene);

		IRenderTechnique* getRenderTechnique() const;
	};

}
