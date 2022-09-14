#pragma once

#include "IRenderTechnique.h"

namespace sa {

	class IRenderLayer {
	protected:
		
	public:
		IRenderLayer() = default;
		virtual ~IRenderLayer() = default;

		virtual void init(RenderWindow* pWindow, IRenderTechnique* pRenderTechnique) = 0;
		virtual void cleanup() {};

		virtual void preRender(RenderContext& context, Camera* pCamera) {};
		virtual void render(RenderContext& context, Camera* pCamera) {};
		virtual void postRender(RenderContext& context) {};

		virtual void onWindowResize(Extent newExtent) {};

	};

	class ImGuiRenderLayer : public IRenderLayer {
	private:
		ResourceID m_imGuiRenderProgram;
		ResourceID m_imGuiFramebuffer;
		RenderWindow* m_pWindow;
	public:
		virtual void init(RenderWindow* pWindow, IRenderTechnique* pRenderTechnique) override;
		virtual void postRender(RenderContext& context) override;

		virtual void onWindowResize(Extent newExtent) override;
	};

	class RenderPipeline {
	private:
		RenderWindow* m_pWindow;
		IRenderTechnique* m_pRenderTechnique;
		RenderContext m_context;
		/*
		ShadowPass m_shadowPass; // inject into render technique before color draw, after culling
		BloomLayer m_bloomPass; // overlay
		ImGuiRenderLayer m_imguiOverlay; // overlay
		*/

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
