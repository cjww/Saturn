#pragma once

#include "Camera.h"
#include "Scene.h"


namespace sa {

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
		
		virtual bool prepareRender(RenderContext& context, Camera* pCamera) { return true; };
		virtual void render(RenderContext& context, Camera* pCamera, ResourceID framebuffer) = 0;
		virtual void compose(RenderContext& context, ResourceID framebuffer) {};

		virtual ResourceID createColorFramebuffer(const Texture2D& outputTexture) = 0;

		virtual void updateLights(Scene* pScene) = 0;
		virtual void collectMeshes(Scene* pScene) = 0;

	};
}