#pragma once

#include <Renderer.hpp>
#include <RenderWindow.hpp>
#include "Assets\IAsset.h"

namespace sa {

	class Engine;

	class RenderTarget : public IAsset {
	private:
		Renderer& m_renderer;

		bool m_isActive;

		Extent m_extent;

		struct MainRenderData {
			DynamicTexture colorTexture;
			DynamicTexture depthTexture;
			ResourceID colorFramebuffer = NULL_RESOURCE;
			ResourceID depthFramebuffer = NULL_RESOURCE;
			

			// Light culling
			glm::uvec2 tileCount;
			DynamicBuffer lightIndexBuffer;
			ResourceID lightCullingDescriptorSet = NULL_RESOURCE;

			bool isInitialized = false;
		} m_mainRenderData;
		
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

		} m_bloomData;

		DynamicTexture* m_pOutputTexture = nullptr;


		friend class BloomRenderLayer;
		void initializeBloomData(RenderContext& context, Extent extent, const DynamicTexture* colorTexture, const ShaderSet& bloomShader, ResourceID sampler);
		void cleanupBloomData();

		friend class ForwardPlus;
		void initializeMainRenderData(ResourceID colorRenderProgram, ResourceID depthPreRenderProgram, 
			const ShaderSet& lightCullingShader,
			ResourceID sampler, Extent extent);
		void cleanupMainRenderData();

		void setOutputTexture(const DynamicTexture& dynamicTexture);

	public:

		RenderTarget();
		RenderTarget(const AssetHeader& header);

		virtual ~RenderTarget();

		void initialize(Extent extent);
		void initialize(Engine* pEngine, RenderWindow* pWindow);
		void destroy();

		void resize(Extent extent);

		void swap();

		bool isReady() const;

		const MainRenderData& getMainRenderData() const;
		const BloomData& getBloomData() const;

		const Extent& getExtent() const;

		const Texture& getOutputTexture() const;

		void setActive(bool isActive);
		bool isActive() const;

		virtual bool onLoad(std::ifstream& file, AssetLoadFlags flags) override;
		virtual bool onWrite(std::ofstream& file, AssetWriteFlags flags) override;
		virtual bool onUnload() override;

	};
}