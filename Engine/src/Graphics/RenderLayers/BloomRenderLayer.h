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
		
		ResourceID m_sampler;

		Extent m_threadCountStack[7];
		uint8_t m_stackSize;

		bool m_wasResized;
		
	public:
		virtual void init() override;
	
		virtual void cleanup() override;
		
		virtual const Texture& render(RenderContext& context, SceneCamera* pCamera, RenderTarget* rendertarget, SceneCollection& sceneCollection) override;
		
		const BloomPreferences& getBloomPreferences() const;
		void setBloomPreferences(const BloomPreferences& bloomPreferences);


	};
}

