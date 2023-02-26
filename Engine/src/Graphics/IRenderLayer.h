#pragma once

#include "Renderer.hpp"
#include "RenderWindow.hpp"
#include "SceneCamera.h"

#include "IRenderTechnique.h"
#include "RenderTarget.h"

namespace sa {

	class IRenderLayer {
	protected:
		Renderer& m_renderer;
		bool m_isActive;
	public:
		IRenderLayer();
		virtual ~IRenderLayer() = default;

		virtual void init(RenderWindow* pWindow, IRenderTechnique* pRenderTechnique = nullptr) = 0;
		virtual void cleanup() = 0;

		virtual void preRender(RenderContext& context, SceneCamera* pCamera, RenderTarget* rendertarget, SceneCollection& sceneCollection) {};
		virtual void render(RenderContext& context, SceneCamera* pCamera, RenderTarget* rendertarget, SceneCollection& sceneCollection) {};
		virtual void postRender(RenderContext& context) {};

		virtual void onWindowResize(Extent newExtent) = 0;

		virtual const Texture2D& getOutputTexture() const = 0;

		bool isActive() const;
		void setActive(bool active);


	};
}
