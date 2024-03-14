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
	
	struct BloomData {
		bool isInitialized = false;

		ResourceID filterDescriptorSet = NULL_RESOURCE;
		std::vector<ResourceID> blurDescriptorSets;
		std::vector<ResourceID> upsampleDescriptorSets;
		ResourceID compositeDescriptorSet = NULL_RESOURCE;

		DynamicTexture2D bloomTexture;
		std::vector<DynamicTexture2D> bloomMipTextures;

		DynamicTexture2D bufferTexture;
		std::vector<DynamicTexture2D> bufferMipTextures;

		DynamicTexture2D outputTexture;

	};

	class BloomRenderLayer : public IRenderLayer<BloomData, BloomPreferences>{
	public:
	private:
		
		ShaderSet m_bloomShader;
		ResourceID m_bloomPipeline;

		Buffer m_bloomPreferencesBuffer;
		BloomPreferences m_bloomPreferences = {};
		ResourceID m_bloomPreferencesDescriptorSet;
		
		ResourceID m_sampler;

		Extent m_threadCountStack[7];
		uint8_t m_stackSize;

		bool m_wasResized;
		

		void initializeBloomData(const UUID& renderTargetID, RenderContext& context, Extent extent, const DynamicTexture* colorTexture);
		void cleanupBloomData(const UUID& renderTargetID);

	public:

		virtual void init() override;
		virtual void cleanup() override;

		virtual void onRenderTargetResize(UUID renderTargetID, Extent oldExtent, Extent newExtent) override;

		virtual bool preRender(RenderContext& context, SceneCollection& sceneCollection) override;
		virtual bool render(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sceneCollection) override;
		virtual bool postRender(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sceneCollection) override;

		void setBloomPreferences(const BloomPreferences& bloomPreferences);

	};
}

