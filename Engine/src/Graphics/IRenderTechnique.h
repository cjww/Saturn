#pragma once

#include "Renderer.hpp"
#include "Resources\Texture.hpp"

#include "SceneCamera.h"
#include "RenderTarget.h"
#include "SceneCollection.h"

namespace sa {

	struct DrawData {
		Texture colorTexture;
		Texture finalTexture;
	};

	class IRenderTechnique {
	public:
		DrawData drawData;

		virtual void init() = 0;
		virtual void cleanup() = 0;

		virtual bool preRender(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sceneCollection) = 0;
		virtual const Texture& render(RenderContext& context, SceneCamera* pCamera, RenderTarget* rendertarget, SceneCollection& sceneCollection) = 0;


	};
}