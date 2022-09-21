#pragma once

#include "IRenderTechnique.h"
namespace sa {

	class RenderPipeline {
	private:
		RenderWindow* m_pWindow;
		IRenderTechnique* m_pRenderTechnique;
		RenderContext m_context;

		std::vector<IRenderLayer*> m_layers;

		tf::Executor m_executor;

		std::set<Camera*> m_cameras;

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
