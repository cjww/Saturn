#pragma once

#include "SceneCamera.h"
#include "Scene.h"
#include "Renderer.hpp"
#include "Resources\Texture.hpp"

#include "Graphics\RenderTarget.h"
#include "SceneCollection.h"

namespace sa {

	struct DrawData {
		Texture colorTexture;
		Texture finalTexture;
	};

	class IRenderTechnique {
	protected:
		Renderer& m_renderer;


	public:
		DrawData drawData;


		IRenderTechnique();
		virtual ~IRenderTechnique() = default;

		virtual void init() = 0;
		virtual void cleanup() = 0;

		virtual void onWindowResize(Extent extent) = 0;

		virtual void updateData(RenderContext& context) = 0;
		
		virtual bool prepareRender(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sceneCollection) { return true; };
		virtual void render(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sceneCollection) = 0;
		virtual void endRender(RenderContext& context) {};

	};
}