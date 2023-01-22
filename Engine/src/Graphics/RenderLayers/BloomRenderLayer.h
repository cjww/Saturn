#pragma once
#include "Graphics/IRenderLayer.h"

#include "Tools/Profiler.h"

namespace sa {

	void generateGaussianKernel(GaussianData& gaussData);

	class BloomRenderLayer : public IRenderLayer {
	private:
		
		ResourceID m_bloomPipeline;

		Buffer m_bloomInfoBuffer;
		BloomInfo m_bloomInfo = {};
		ResourceID m_bloomInfoDescriptorSet;
		
		IRenderTechnique* m_pRenderTechnique;

		ResourceID m_sampler;

		Extent m_threadCountStack[7];
		uint8_t m_stackSize;


		bool m_wasResized;
		
		void cleanupBloomData(RenderTarget::BloomData& bd);
		void initializeBloomData(RenderContext& context, Extent extent, DynamicTexture* colorTexture, RenderTarget::BloomData& bd);

	public:
		virtual void init(RenderWindow* pWindow, IRenderTechnique* pRenderTechnique = nullptr) override;
	
		virtual void cleanup() override;
		virtual void onWindowResize(Extent newExtent) override;

		virtual void render(RenderContext& context, SceneCamera* pCamera, RenderTarget* rendertarget) override;
		
		virtual const Texture2D& getOutputTexture() const override;

	};
}

