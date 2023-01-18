#pragma once
#include "Graphics/IRenderLayer.h"

#include "Tools/Profiler.h"

namespace sa {
	class BloomRenderLayer : public IRenderLayer {
	private:
		ResourceID m_filterPipeline;
		ResourceID m_blurComputePipeline;
		ResourceID m_upsamplePipeline;
		ResourceID m_compositePipeline;

		IRenderTechnique* m_pRenderTechnique;
		
		ResourceID m_sampler;

		Extent m_threadCountStack[7];
		uint8_t m_stackSize;

		bool m_wasResized;

	public:
		virtual void init(RenderWindow* pWindow, IRenderTechnique* pRenderTechnique = nullptr) override;
	
		virtual void cleanup() override;
		virtual void onWindowResize(Extent newExtent) override;

		virtual void render(RenderContext& context, SceneCamera* pCamera, RenderTarget* rendertarget) override;
		
		virtual const Texture2D& getOutputTexture() const override;

	};
}

