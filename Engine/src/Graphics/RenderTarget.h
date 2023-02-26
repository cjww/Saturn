#pragma once

#include <Renderer.hpp>
#include <RenderWindow.hpp>

namespace sa {

	class Engine;

	class RenderTarget {
	private:
		Renderer& m_renderer;

		friend class BloomRenderLayer;
		void initializeBloomData(RenderContext& context, Extent extent, DynamicTexture* colorTexture, ResourceID bloomPipeline, ResourceID sampler);
		void cleanupBloomData();

		friend class ForwardPlus;
		void cleanupMainRenderData();

	public:

		Extent extent;

		struct MainRenderData {
			DynamicTexture colorTexture;
			DynamicTexture depthTexture;
			ResourceID colorFramebuffer = NULL_RESOURCE;
			ResourceID colorPipeline = NULL_RESOURCE;
			ResourceID sceneDescriptorSet = NULL_RESOURCE;
			
			ResourceID depthPipeline = NULL_RESOURCE;
			ResourceID depthFramebuffer = NULL_RESOURCE;
			ResourceID sceneDepthDescriptorSet = NULL_RESOURCE;


			// Light culling
			glm::uvec2 tileCount;
			DynamicBuffer lightIndexBuffer;
			ResourceID lightCullingDescriptorSet = NULL_RESOURCE;

			bool isInitialized = false;
		} mainRenderData;
		
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
		virtual ~RenderTarget();

		void initialize(Extent extent);
		void initialize(Engine* pEngine, RenderWindow* pWindow);
		void destroy();

		void resize(Extent extent);

		void swap();

		bool isReady() const;

	};
}