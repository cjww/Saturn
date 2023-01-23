#include "pch.h"
#include "RenderTarget.h"
#include "IRenderTechnique.h"

#include "Engine.h"

namespace sa {
	void RenderTarget::initializeMainData(IRenderTechnique* pRenderTechnique) {
		colorTexture = pRenderTechnique->createColorAttachmentTexture(extent);
		depthTexture = pRenderTechnique->createDepthAttachmentTexture(extent);
		framebuffer = pRenderTechnique->createColorFramebuffer(colorTexture, depthTexture);
	}

	void RenderTarget::cleanupMainData() {
		if(colorTexture.isValid())
			colorTexture.destroy();
		if (depthTexture.isValid())
			depthTexture.destroy();
		if (framebuffer != NULL_RESOURCE) {
			m_renderer.destroyFramebuffer(framebuffer);
			framebuffer = NULL_RESOURCE;
		}
	}

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

	RenderTarget::RenderTarget()
		: m_renderer(Renderer::get())
		, m_pEngine(nullptr)
	{

	}

	void RenderTarget::initialize(Engine* pEngine, Extent extent) {
		m_pEngine = pEngine;
		this->extent = extent;
		/*
		colorTexture = DynamicTexture2D(TextureTypeFlagBits::COLOR_ATTACHMENT | TextureTypeFlagBits::SAMPLED, extent);
		depthTexture = DynamicTexture2D(TextureTypeFlagBits::DEPTH_ATTACHMENT, extent);
		framebuffer = pEngine->getRenderPipeline().getRenderTechnique()->createColorFramebuffer(colorTexture, depthTexture);
		*/
		
		isInitialized = false; // initialize main data in main render pass
		bloomData.isInitialized = false; // initialize bloom data in bloom pass
	}
	
	void RenderTarget::initialize(Engine* pEngine, RenderWindow* pWindow) {
		pEngine->on<engine_event::WindowResized>([this](engine_event::WindowResized& e, Engine& engine) {
			this->resize(e.newExtent);
			this->extent = e.newExtent;
		});
		
		initialize(pEngine, pWindow->getCurrentExtent());
	}

	void RenderTarget::destroy() {
		
	}

	void RenderTarget::resize(Extent extent) {
		destroy();
		initialize(m_pEngine, extent);
	}

}