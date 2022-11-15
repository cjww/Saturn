#pragma once

#include "SceneCamera.h"
#include "Scene.h"
#include "Renderer.hpp"
#include "Resources\Texture.hpp"

namespace sa {

	struct RenderTarget {
		ResourceID framebuffer;
		ResourceID renderProgram;
		ResourceID pipeline;
	};

	struct DrawData {
		Texture2D colorTexture;
		Texture2D finalTexture;
	};

	class IRenderTechnique {
	protected:
		Renderer& m_renderer;


	public:
		DrawData drawData;


		IRenderTechnique();
		virtual ~IRenderTechnique() = default;

		virtual void init(Extent extent) = 0;
		virtual void cleanup() = 0;

		virtual void onWindowResize(Extent extent) = 0;

		virtual void updateData(RenderContext& context) = 0;
		
		virtual bool prepareRender(RenderContext& context, SceneCamera* pCamera) { return true; };
		virtual void render(RenderContext& context, SceneCamera* pCamera, ResourceID framebuffer) = 0;
		virtual void endRender(RenderContext& context) {};

		virtual ResourceID createColorFramebuffer(const Texture2D& outputTexture) = 0;

		virtual void updateLights(Scene* pScene) = 0;
		virtual void collectMeshes(Scene* pScene) = 0;

	};
}