#pragma once

#include "Camera.h"
#include "Scene.h"

#include "Renderer.hpp"
#include "RenderWindow.hpp"



namespace sa {
	class IRenderTechnique {
	protected:
		Renderer& m_renderer;

		bool m_isRenderingToSwapchain;

		sa::RenderWindow* m_pWindow;

		sa::Texture2D m_outputTexture;

	public:
		IRenderTechnique(bool renderToSwapchain = true);
		virtual ~IRenderTechnique() = default;

		virtual void onWindowResize(Extent extent) = 0;

		virtual void init(sa::RenderWindow* pWindow) = 0;
		virtual void cleanup() = 0;

		virtual void updateData(RenderContext& context, Scene* pScene) = 0;
		virtual void preRender(RenderContext& context, Camera* pCamera) = 0;
		virtual void render(RenderContext& context, Camera* pCamera) = 0;
		virtual void postRender(RenderContext& context) = 0;

		virtual const Texture& getOutputTexture() const;

		virtual void updateLights(Scene* pScene) = 0;

		sa::Extent getCurrentExtent() const;
		
	};
}