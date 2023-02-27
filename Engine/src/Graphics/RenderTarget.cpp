#include "pch.h"
#include "RenderTarget.h"
#include "IRenderTechnique.h"

#include "Engine.h"

namespace sa {

	void RenderTarget::initializeBloomData(RenderContext& context, Extent extent, DynamicTexture* colorTexture, ResourceID bloomPipeline, ResourceID sampler) {
		//Textures
		bloomData.bloomTexture = DynamicTexture2D(TextureTypeFlagBits::STORAGE | TextureTypeFlagBits::SAMPLED, extent, 1U, 6U);
		bloomData.bloomMipTextures = bloomData.bloomTexture.createMipLevelTextures();

		bloomData.bufferTexture = DynamicTexture2D(TextureTypeFlagBits::STORAGE | TextureTypeFlagBits::SAMPLED, extent, 1U, bloomData.bloomMipTextures.size() - 1);
		bloomData.bufferMipTextures = bloomData.bufferTexture.createMipLevelTextures();

		//bloomData.outputTexture = DynamicTexture2D(TextureTypeFlagBits::STORAGE | TextureTypeFlagBits::SAMPLED, tex.getExtent(), sa::FormatPrecisionFlagBits::e8Bit, sa::FormatDimensionFlagBits::e4, sa::FormatTypeFlagBits::UNORM);
		bloomData.outputTexture = DynamicTexture2D(TextureTypeFlagBits::STORAGE | TextureTypeFlagBits::SAMPLED, colorTexture->getExtent());

		// DescriptorSets
		if (bloomData.filterDescriptorSet == NULL_RESOURCE)
			bloomData.filterDescriptorSet = m_renderer.allocateDescriptorSet(bloomPipeline, 0);

		if (bloomData.blurDescriptorSets.empty()) {
			bloomData.blurDescriptorSets.resize(bloomData.bloomMipTextures.size() - 1);
			for (size_t i = 0; i < bloomData.blurDescriptorSets.size(); i++) {
				bloomData.blurDescriptorSets[i] = m_renderer.allocateDescriptorSet(bloomPipeline, 0);
			}
		}

		if (bloomData.upsampleDescriptorSets.empty()) {
			bloomData.upsampleDescriptorSets.resize(bloomData.bloomMipTextures.size() - 1);
			for (size_t i = 0; i < bloomData.upsampleDescriptorSets.size(); i++) {
				bloomData.upsampleDescriptorSets[i] = m_renderer.allocateDescriptorSet(bloomPipeline, 0);
			}
		}

		if (bloomData.compositeDescriptorSet == NULL_RESOURCE)
			bloomData.compositeDescriptorSet = m_renderer.allocateDescriptorSet(bloomPipeline, 0);



		for (int i = 0; i < bloomData.bloomTexture.getTextureCount(); i++) {
			context.transitionTexture(bloomData.bloomTexture.getTexture(i), sa::Transition::NONE, sa::Transition::COMPUTE_SHADER_WRITE);
			context.transitionTexture(bloomData.bufferTexture.getTexture(i), sa::Transition::NONE, sa::Transition::COMPUTE_SHADER_WRITE);
			context.transitionTexture(bloomData.outputTexture.getTexture(i), sa::Transition::NONE, sa::Transition::COMPUTE_SHADER_WRITE);
		}

		m_renderer.updateDescriptorSet(bloomData.filterDescriptorSet, 0, *colorTexture, sampler);
		m_renderer.updateDescriptorSet(bloomData.filterDescriptorSet, 1, bloomData.bloomMipTextures[0]);
		m_renderer.updateDescriptorSet(bloomData.filterDescriptorSet, 2, bloomData.bloomMipTextures[0]);
		m_renderer.updateDescriptorSet(bloomData.filterDescriptorSet, 3, bloomData.bloomMipTextures[0]);
		for (size_t i = 0; i < bloomData.bloomMipTextures.size() - 1; i++) {
			m_renderer.updateDescriptorSet(bloomData.blurDescriptorSets[i], 0, bloomData.bloomMipTextures[i], sampler);
			m_renderer.updateDescriptorSet(bloomData.blurDescriptorSets[i], 1, bloomData.bloomMipTextures[i]);
			m_renderer.updateDescriptorSet(bloomData.blurDescriptorSets[i], 2, bloomData.bloomMipTextures[i]);
			m_renderer.updateDescriptorSet(bloomData.blurDescriptorSets[i], 3, bloomData.bloomMipTextures[i + 1]);
		}

		DynamicTexture2D smallImage = bloomData.bloomMipTextures[bloomData.bloomMipTextures.size() - 1];
		DynamicTexture2D bigImage = bloomData.bloomMipTextures[bloomData.bloomMipTextures.size() - 2];
		for (int i = (int)bloomData.bufferMipTextures.size() - 1; i >= 0; i--) {
			m_renderer.updateDescriptorSet(bloomData.upsampleDescriptorSets[i], 0, *colorTexture, sampler);
			m_renderer.updateDescriptorSet(bloomData.upsampleDescriptorSets[i], 1, smallImage);
			m_renderer.updateDescriptorSet(bloomData.upsampleDescriptorSets[i], 2, bigImage);
			m_renderer.updateDescriptorSet(bloomData.upsampleDescriptorSets[i], 3, bloomData.bufferMipTextures[i]);
			if (i > 0) {
				smallImage = bloomData.bufferMipTextures[i];
				bigImage = bloomData.bloomMipTextures[i - 1];
			}
		}

		m_renderer.updateDescriptorSet(bloomData.compositeDescriptorSet, 0, *colorTexture, sampler);
		m_renderer.updateDescriptorSet(bloomData.compositeDescriptorSet, 1, bloomData.bufferMipTextures[0]);
		m_renderer.updateDescriptorSet(bloomData.compositeDescriptorSet, 2, bloomData.bufferMipTextures[0]);
		m_renderer.updateDescriptorSet(bloomData.compositeDescriptorSet, 3, bloomData.outputTexture);
	}

	void RenderTarget::cleanupBloomData() {
		if (bloomData.bloomTexture.isValid()) {
			for (auto& tex : bloomData.bloomMipTextures) {
				tex.destroy();
			}
			bloomData.bloomTexture.destroy();
		}
		if (bloomData.bufferTexture.isValid()) {
			for (auto& tex : bloomData.bufferMipTextures) {
				tex.destroy();
			}
			bloomData.bufferTexture.destroy();
		}
		if (bloomData.outputTexture.isValid())
			bloomData.outputTexture.destroy();
	}

	void RenderTarget::cleanupMainRenderData() {
		if (mainRenderData.colorTexture.isValid())
			mainRenderData.colorTexture.destroy();
		if (mainRenderData.depthTexture.isValid())
			mainRenderData.depthTexture.destroy();

		if (mainRenderData.depthFramebuffer != NULL_RESOURCE) {
			m_renderer.destroyFramebuffer(mainRenderData.depthFramebuffer);
			mainRenderData.depthFramebuffer = NULL_RESOURCE;
		}

		if (mainRenderData.sceneDepthDescriptorSet != NULL_RESOURCE) {
			m_renderer.freeDescriptorSet(mainRenderData.sceneDepthDescriptorSet);
			mainRenderData.sceneDepthDescriptorSet = NULL_RESOURCE;
		}

		if (mainRenderData.depthPipeline != NULL_RESOURCE) {
			m_renderer.destroyPipeline(mainRenderData.depthPipeline);
			mainRenderData.depthPipeline = NULL_RESOURCE;
		}

		if (mainRenderData.lightIndexBuffer.isValid())
			mainRenderData.lightIndexBuffer.destroy();

		if (mainRenderData.colorFramebuffer != NULL_RESOURCE) {
			m_renderer.destroyFramebuffer(mainRenderData.colorFramebuffer);
			mainRenderData.colorFramebuffer = NULL_RESOURCE;
		}

		if (mainRenderData.sceneDescriptorSet != NULL_RESOURCE) {
			m_renderer.freeDescriptorSet(mainRenderData.sceneDescriptorSet);
			mainRenderData.sceneDescriptorSet = NULL_RESOURCE;
		}
		if (mainRenderData.colorPipeline != NULL_RESOURCE) {
			m_renderer.destroyPipeline(mainRenderData.colorPipeline);
			mainRenderData.colorPipeline = NULL_RESOURCE;
		}
	}

	RenderTarget::RenderTarget(const AssetHeader& header) 
		: IAsset(header)
		, m_renderer(Renderer::get())
	{

	}

	RenderTarget::RenderTarget() 
		: RenderTarget(AssetHeader{})
	{
	}


	RenderTarget::~RenderTarget() {
		destroy();
	}

	void RenderTarget::initialize(Extent extent) {
		this->extent = extent;
		
		mainRenderData.isInitialized = false; // initialize main data in main render pass
		bloomData.isInitialized = false; // initialize bloom data in bloom pass
	}
	
	void RenderTarget::initialize(Engine* pEngine, RenderWindow* pWindow) {
		pEngine->on<engine_event::WindowResized>([this](engine_event::WindowResized& e, Engine& engine) {
			this->resize(e.newExtent);
			this->extent = e.newExtent;
		});
		
		initialize(pWindow->getCurrentExtent());
	}

	void RenderTarget::destroy() {
		cleanupMainRenderData();
		cleanupBloomData();
	}

	void RenderTarget::resize(Extent extent) {
		this->extent = extent;

		mainRenderData.isInitialized = false; // initialize main data in main render pass
		bloomData.isInitialized = false; // initialize bloom data in bloom pass

		this->outputTexture = nullptr;
	}

	void RenderTarget::swap() {
		mainRenderData.colorTexture.swap();
		mainRenderData.depthTexture.swap();
		mainRenderData.lightIndexBuffer.swap();

		m_renderer.swapFramebuffer(mainRenderData.colorFramebuffer);
		m_renderer.swapFramebuffer(mainRenderData.depthFramebuffer);

		bloomData.outputTexture.swap();

	}

	bool RenderTarget::isReady() const {
		return
			outputTexture != nullptr &&
			outputTexture->isValid() &&
			mainRenderData.isInitialized;
	}

	bool RenderTarget::onLoad(std::ifstream& file, AssetLoadFlags flags) {
		return true;
	}
	
	bool RenderTarget::onWrite(std::ofstream& file, AssetWriteFlags flags) {
		return true;
	}

	bool RenderTarget::onUnload() {
		return true;
	}

}