#include "pch.h"
#include "BloomRenderLayer.h"

namespace sa {

	void BloomRenderLayer::cleanupBloomData(RenderTarget::BloomData& bd) {

		if (bd.bloomTexture.isValid()) {
			for (auto& tex : bd.bloomMipTextures) {
				tex.destroy();
			}
			bd.bloomTexture.destroy();
		}
		if (bd.bufferTexture.isValid()) {
			for (auto& tex : bd.bufferMipTextures) {
				tex.destroy();
			}
			bd.bufferTexture.destroy();
		}
		if (bd.outputTexture.isValid())
			bd.outputTexture.destroy();

		// DescriptorSets
		if (bd.filterDescriptorSet != NULL_RESOURCE)
			m_renderer.freeDescriptorSet(bd.filterDescriptorSet);

		for (size_t i = 0; i < bd.blurDescriptorSets.size(); i++) {
			m_renderer.freeDescriptorSet(bd.blurDescriptorSets[i]);
		}
		for (size_t i = 0; i < bd.upsampleDescriptorSets.size(); i++) {
			m_renderer.freeDescriptorSet(bd.upsampleDescriptorSets[i]);
		}

		if (bd.compositeDescriptorSet != NULL_RESOURCE)
			m_renderer.freeDescriptorSet(bd.compositeDescriptorSet);
	}

	void BloomRenderLayer::initializeBloomData(RenderContext& context, Extent extent, DynamicTexture* colorTexture, RenderTarget::BloomData& bd) {

		//Textures
		bd.bloomTexture = DynamicTexture2D(TextureTypeFlagBits::STORAGE | TextureTypeFlagBits::SAMPLED, extent, 1U, 6U);
		bd.bloomMipTextures = bd.bloomTexture.createMipLevelTextures();

		bd.bufferTexture = DynamicTexture2D(TextureTypeFlagBits::STORAGE | TextureTypeFlagBits::SAMPLED, extent, 1U, bd.bloomMipTextures.size() - 1);
		bd.bufferMipTextures = bd.bufferTexture.createMipLevelTextures();

		//bd.outputTexture = DynamicTexture2D(TextureTypeFlagBits::STORAGE | TextureTypeFlagBits::SAMPLED, tex.getExtent(), sa::FormatPrecisionFlagBits::e8Bit, sa::FormatDimensionFlagBits::e4, sa::FormatTypeFlagBits::UNORM);
		bd.outputTexture = DynamicTexture2D(TextureTypeFlagBits::STORAGE | TextureTypeFlagBits::SAMPLED, colorTexture->getExtent());

		// DescriptorSets
		bd.filterDescriptorSet = m_renderer.allocateDescriptorSet(m_filterPipeline, 0);

		bd.blurDescriptorSets.resize(bd.bloomMipTextures.size() - 1);
		for (size_t i = 0; i < bd.blurDescriptorSets.size(); i++) {
			bd.blurDescriptorSets[i] = m_renderer.allocateDescriptorSet(m_blurComputePipeline, 0);
		}

		bd.upsampleDescriptorSets.resize(bd.bloomMipTextures.size() - 1);
		for (size_t i = 0; i < bd.upsampleDescriptorSets.size(); i++) {
			bd.upsampleDescriptorSets[i] = m_renderer.allocateDescriptorSet(m_upsamplePipeline, 0);
		}
		bd.compositeDescriptorSet = m_renderer.allocateDescriptorSet(m_compositePipeline, 0);

		for (int i = 0; i < bd.bloomTexture.getTextureCount(); i++) {
			context.transitionTexture(bd.bloomTexture.getTexture(i), sa::Transition::NONE, sa::Transition::COMPUTE_SHADER_WRITE);
			context.transitionTexture(bd.bufferTexture.getTexture(i), sa::Transition::NONE, sa::Transition::COMPUTE_SHADER_WRITE);
			context.transitionTexture(bd.outputTexture.getTexture(i), sa::Transition::NONE, sa::Transition::COMPUTE_SHADER_WRITE);
		}

		m_renderer.updateDescriptorSet(bd.filterDescriptorSet, 0, *colorTexture, m_sampler);
		m_renderer.updateDescriptorSet(bd.filterDescriptorSet, 1, bd.bloomMipTextures[0]);
		for (size_t i = 0; i < bd.bloomMipTextures.size() - 1; i++) {
			m_renderer.updateDescriptorSet(bd.blurDescriptorSets[i], 0, bd.bloomMipTextures[i]);
			m_renderer.updateDescriptorSet(bd.blurDescriptorSets[i], 1, bd.bloomMipTextures[i + 1]);
		}

		DynamicTexture2D smallImage = bd.bloomMipTextures[bd.bloomMipTextures.size() - 1];
		DynamicTexture2D bigImage = bd.bloomMipTextures[bd.bloomMipTextures.size() - 2];
		for (int i = (int)bd.bufferMipTextures.size() - 1; i >= 0; i--) {
			m_renderer.updateDescriptorSet(bd.upsampleDescriptorSets[i], 0, smallImage);
			m_renderer.updateDescriptorSet(bd.upsampleDescriptorSets[i], 1, bigImage);
			m_renderer.updateDescriptorSet(bd.upsampleDescriptorSets[i], 2, bd.bufferMipTextures[i]);
			if (i > 0) {
				smallImage = bd.bufferMipTextures[i];
				bigImage = bd.bloomMipTextures[i - 1];
			}
		}

		m_renderer.updateDescriptorSet(bd.compositeDescriptorSet, 0, *colorTexture, m_sampler);
		m_renderer.updateDescriptorSet(bd.compositeDescriptorSet, 1, bd.bufferMipTextures[0]);
		m_renderer.updateDescriptorSet(bd.compositeDescriptorSet, 2, bd.bufferMipTextures[0]);
		m_renderer.updateDescriptorSet(bd.compositeDescriptorSet, 3, bd.outputTexture);
	}

	void BloomRenderLayer::init(RenderWindow* pWindow, IRenderTechnique* pRenderTechnique) {

		m_pRenderTechnique = pRenderTechnique;

		m_filterPipeline = m_renderer.createComputePipeline("../Engine/shaders/Filter.comp.spv");
		m_blurComputePipeline = m_renderer.createComputePipeline("../Engine/shaders/GaussianBlur.comp.spv");
		m_upsamplePipeline = m_renderer.createComputePipeline("../Engine/shaders/Upsample.comp.spv");
		m_compositePipeline = m_renderer.createComputePipeline("../Engine/shaders/CompositeBloom.comp.spv");
		
		SamplerInfo samplerInfo = {};
		samplerInfo.minFilter = FilterMode::LINEAR;
		samplerInfo.magFilter = FilterMode::LINEAR;
		
		samplerInfo.addressModeU = SamplerAddressMode::CLAMP_TO_EDGE;
		samplerInfo.addressModeV = SamplerAddressMode::CLAMP_TO_EDGE;
		samplerInfo.addressModeW = SamplerAddressMode::CLAMP_TO_EDGE;

		m_sampler = m_renderer.createSampler(samplerInfo);

		m_stackSize = 0;
	}

	void BloomRenderLayer::cleanup() {

	}

	void BloomRenderLayer::onWindowResize(Extent newExtent) {
		
	}

	void BloomRenderLayer::render(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget) {
		SA_PROFILE_FUNCTION();

		RenderTarget::BloomData& bd = pRenderTarget->bloomData;

		DynamicTexture* tex = m_renderer.getFramebufferDynamicTexturePtr(pRenderTarget->framebuffer, 0);
		Extent extent = { std::ceil(tex->getExtent().width * 0.5f), std::ceil(tex->getExtent().height * 0.5f) };
		
		if (!bd.isInitialized) {
			bd.isInitialized = true;
			// Free old data
			cleanupBloomData(bd);
			
			// Initialize
			initializeBloomData(context, extent, tex, bd);
		}
		

		uint32_t threadX = std::ceil(extent.width / 32.f);
		uint32_t threadY = std::ceil(extent.height / 32.f);
		m_threadCountStack[m_stackSize++] = { threadX << 1, threadY << 1 };
		m_threadCountStack[m_stackSize++] = { threadX, threadY };

		// Filter
		context.bindPipeline(m_filterPipeline);
		context.bindDescriptorSet(bd.filterDescriptorSet, m_filterPipeline);
		context.dispatch(threadX, threadY, 1);

		
		// Downsample + blur
		context.bindPipeline(m_blurComputePipeline);
		for (size_t i = 0; i < bd.bloomMipTextures.size() - 1; i++) {
			threadX += 1 * threadX % 2;
			threadX = threadX >> 1;

			threadY += 1 * threadY % 2;
			threadY = threadY >> 1;
			m_threadCountStack[m_stackSize++] = { threadX, threadY };

			context.bindDescriptorSet(bd.blurDescriptorSets[i], m_blurComputePipeline);
			context.dispatch(threadX, threadY, 1);
		}

		// Upsample + combine

		context.bindPipeline(m_upsamplePipeline);
		m_stackSize--;
		for (int i = (int)bd.bufferMipTextures.size() - 1; i >= 0; i--) {
			m_stackSize--;
			threadX = m_threadCountStack[m_stackSize].width;
			threadY = m_threadCountStack[m_stackSize].height;

			context.bindDescriptorSet(bd.upsampleDescriptorSets[i], m_upsamplePipeline);
			context.dispatch(threadX, threadY, 1);
			
		}

		threadX = m_threadCountStack[--m_stackSize].width;
		threadY = m_threadCountStack[m_stackSize].height;

		// Composite + Tonemap
		context.bindPipeline(m_compositePipeline);
		
		context.bindDescriptorSet(bd.compositeDescriptorSet, m_compositePipeline);
		context.dispatch(threadX, threadY, 1);

		m_pRenderTechnique->drawData.finalTexture = bd.outputTexture;
		pRenderTarget->outputTexture = &bd.outputTexture;
		
		bd.outputTexture.swap();

	}

	const Texture2D& BloomRenderLayer::getOutputTexture() const {
		return {};
	}
}
