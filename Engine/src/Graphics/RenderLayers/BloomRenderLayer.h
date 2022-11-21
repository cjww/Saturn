#pragma once
#include "Graphics/IRenderLayer.h"

namespace sa {
	class BloomRenderLayer : public IRenderLayer {
	private:
		ResourceID m_filterPipeline;
		ResourceID m_blurComputePipeline;
		ResourceID m_upsamplePipeline;
		ResourceID m_compositePipeline;

		IRenderTechnique* m_pRenderTechnique;
		
		ResourceID m_sampler;

		bool m_wasResized;

	public:
		virtual void init(RenderWindow* pWindow, IRenderTechnique* pRenderTechnique = nullptr) override;
	
		virtual void cleanup() override;
		virtual void onWindowResize(Extent newExtent) override;

		virtual void render(RenderContext& context, SceneCamera* pCamera, RenderTarget* rendertarget) override;

		virtual const Texture2D& getOutputTexture() const override;

	};
}

