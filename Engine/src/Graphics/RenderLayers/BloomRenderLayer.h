#pragma once
#include "Graphics/IRenderLayer.h"

#include "Tools/Profiler.h"

namespace sa {

	
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

