#pragma once
#include "Graphics/IRenderLayer.h"

#include "Tools/Profiler.h"

namespace sa {

	struct GaussianData {
		float normFactor;
		int kernelRadius = 3;
		alignas(16) glm::vec4 kernel[13];
	};
	void generateGaussianKernel(GaussianData& gaussData);

	struct TonemapPreferences {
		float gamma = 2.2f;
		float exposure = 1.0f;
		int tonemappingAlgorithm = 0;
	};

	struct BloomPreferences {
		float threshold = 1.0f;
		float intensity = 1.0f;
		float spread = 1.0f;
		alignas(16) TonemapPreferences tonemapPreferences = {};
		alignas(16) GaussianData gaussData = {};
	};
	

	class BloomRenderLayer : public IRenderLayer {
	private:
		
		ResourceID m_bloomPipeline;

		Buffer m_bloomPreferencesBuffer;
		BloomPreferences m_bloomPreferences = {};
		ResourceID m_bloomPreferencesDescriptorSet;
		
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

		const BloomPreferences& getBloomPreferences() const;
		void setBloomPreferences(const BloomPreferences& bloomPreferences);


	};
}

