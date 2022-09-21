#pragma once

#include "Camera.h"
#include "Scene.h"


#include "IRenderLayer.h"


namespace sa {
	class IRenderTechnique : public IRenderLayer {
	protected:
		bool m_isRenderingToSwapchain;

		sa::RenderWindow* m_pWindow;


	public:
		IRenderTechnique(bool renderToSwapchain = true);
		virtual ~IRenderTechnique() = default;

		//virtual void onWindowResize(Extent extent) = 0;

		//virtual void init(sa::RenderWindow* pWindow) = 0;
		virtual void cleanup() = 0;

		virtual void updateData(RenderContext& context) = 0;
		/*
		virtual void preRender(RenderContext& context, Camera* pCamera) = 0;
		virtual void render(RenderContext& context, Camera* pCamera) = 0;
		virtual void postRender(RenderContext& context) = 0;
		*/


		virtual void updateLights(Scene* pScene) = 0;
		virtual void collectMeshes(Scene* pScene) = 0;


		sa::Extent getCurrentExtent() const;
		
	};
}