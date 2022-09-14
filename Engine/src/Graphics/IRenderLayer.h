#pragma once

#include "Renderer.hpp"
#include "RenderWindow.hpp"
#include "Camera.h"

namespace sa {

	class IRenderLayer {
	protected:
		Renderer& m_renderer;
		sa::Texture2D m_outputTexture;

	public:
		IRenderLayer();
		virtual ~IRenderLayer() = default;

		virtual void init(RenderWindow* pWindow, IRenderLayer* pPreviousLayer = nullptr) = 0;
		virtual void cleanup() = 0;

		virtual void preRender(RenderContext& context, Camera* pCamera) {};
		virtual void render(RenderContext& context, Camera* pCamera) {};
		virtual void postRender(RenderContext& context) {};

		virtual void onWindowResize(Extent newExtent) = 0;

		virtual const Texture2D& getOutputTexture() const;

	};
}
