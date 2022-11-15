#pragma once

#include "Renderer.hpp"
#include "RenderWindow.hpp"
#include "SceneCamera.h"

#include "IRenderTechnique.h"

namespace sa {

	class IRenderLayer {
	protected:
		Renderer& m_renderer;
		
	public:
		IRenderLayer();
		virtual ~IRenderLayer() = default;

		virtual void init(RenderWindow* pWindow, IRenderTechnique* pRenderTechnique = nullptr) = 0;
		virtual void cleanup() = 0;

		virtual void preRender(RenderContext& context, SceneCamera* pCamera) {};
		virtual void render(RenderContext& context, SceneCamera* pCamera, RenderTarget* rendertarget = nullptr) {};
		virtual void postRender(RenderContext& context) {};

		virtual void onWindowResize(Extent newExtent) = 0;

		virtual const Texture2D& getOutputTexture() const = 0;

	};
}
