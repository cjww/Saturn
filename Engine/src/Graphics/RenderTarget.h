#pragma once

//#include <Resources\DynamicTexture.hpp>

#include <Renderer.hpp>
#include <RenderWindow.hpp>

namespace sa {

	class Engine;
	class IRenderTechnique;

	class RenderTarget {
	private:
		Renderer& m_renderer;
		Engine* m_pEngine;
		
		friend class MainRenderLayer;
		void initializeMainData(IRenderTechnique* pRenderTechnique);
		void cleanupMainData();


		friend class BloomRenderLayer;
		void initializeBloomData(RenderContext& context, Extent extent, DynamicTexture* colorTexture, ResourceID bloomPipeline, ResourceID sampler);
		void cleanupBloomData();

	public:
		Extent extent;

		DynamicTexture colorTexture;
		DynamicTexture depthTexture;
		ResourceID framebuffer = NULL_RESOURCE;
		ResourceID renderProgram = NULL_RESOURCE;
		ResourceID pipeline = NULL_RESOURCE;
		bool isInitialized = false;
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

		} bloomData;

		DynamicTexture* outputTexture = nullptr;

		RenderTarget();

		void initialize(Engine* pEngine, Extent extent);
		void initialize(Engine* pEngine, RenderWindow* pWindow);
		void destroy();

		void resize(Extent extent);

	};
}