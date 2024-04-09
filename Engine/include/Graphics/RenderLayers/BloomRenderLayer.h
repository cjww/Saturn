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

		DynamicTexture bloomTexture;
		std::vector<DynamicTexture> bloomMipTextures;

		DynamicTexture bufferTexture;
		std::vector<DynamicTexture> bufferMipTextures;

		DynamicTexture outputTexture;

	};

	class BloomRenderLayer : public IRenderLayer<BloomData, BloomPreferences>{
	private:
		Shader m_bloomShader;
		PipelineLayout m_pipelineLayout;
		ResourceID m_bloomPipeline = NULL_RESOURCE;

		Buffer m_bloomPreferencesBuffer;
		ResourceID m_bloomPreferencesDescriptorSet = NULL_RESOURCE;
		
		ResourceID m_sampler = NULL_RESOURCE;

		Extent m_threadCountStack[7];
		uint8_t m_stackSize = 0;
		

		void initializeBloomData(const UUID& renderTargetID, RenderContext& context, Extent extent, const DynamicTexture* colorTexture);
		void cleanupBloomData(const UUID& renderTargetID);

	public:

		virtual void init() override;
		virtual void cleanup() override;

		virtual void onRenderTargetResize(UUID renderTargetID, Extent oldExtent, Extent newExtent) override;
		virtual void onPreferencesUpdated() override;


		virtual bool render(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sceneCollection) override;
		virtual bool postRender(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sceneCollection) override;

	};
}

