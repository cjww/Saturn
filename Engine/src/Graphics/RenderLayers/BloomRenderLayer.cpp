#include "pch.h"
#include "BloomRenderLayer.h"

namespace sa {
	void BloomRenderLayer::init(RenderWindow* pWindow, IRenderTechnique* pRenderTechnique) {

		m_pRenderTechnique = pRenderTechnique;

		m_filterPipeline = m_renderer.createComputePipeline("../Engine/shaders/Filter.comp.spv");
		m_blurComputePipeline = m_renderer.createComputePipeline("../Engine/shaders/GaussianBlur.comp.spv");
		m_upsamplePipeline = m_renderer.createComputePipeline("../Engine/shaders/Upsample.comp.spv");
		m_compositePipeline = m_renderer.createComputePipeline("../Engine/shaders/CompositeBloom.comp.spv");
		
		m_sampler = m_renderer.createSampler(sa::FilterMode::LINEAR);

	}

	void BloomRenderLayer::cleanup() {

	}

	void BloomRenderLayer::onWindowResize(Extent newExtent) {

	}

	void BloomRenderLayer::render(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget) {
		SA_PROFILE_FUNCTION();

		RenderTarget::BloomData& bd = pRenderTarget->bloomData;


		auto tex = m_renderer.getFramebufferTexture(pRenderTarget->framebuffer, 0);
		Extent extent = { std::ceil(tex.getExtent().width * 0.5f), std::ceil(tex.getExtent().height * 0.5f) };
		//Extent extent = tex.getExtent();

		if (!bd.isInitialized) {
			bd.isInitialized = true;
			// Free old data
			//Textures
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
			if(bd.outputTexture.isValid())
				bd.outputTexture.destroy();

			// DescriptorSets
			if(bd.filterDescriptorSet != NULL_RESOURCE)
				m_renderer.freeDescriptorSet(bd.filterDescriptorSet);
				
			for (size_t i = 0; i < bd.blurDescriptorSets.size(); i++) {
				m_renderer.freeDescriptorSet(bd.blurDescriptorSets[i]);
			}
			for (size_t i = 0; i < bd.upsampleDescriptorSets.size(); i++) {
				m_renderer.freeDescriptorSet(bd.upsampleDescriptorSets[i]);
			}
			if(bd.compositeDescriptorSet != NULL_RESOURCE)
				m_renderer.freeDescriptorSet(bd.compositeDescriptorSet);
			

			// Initialize
			//Textures
			bd.bloomTexture = DynamicTexture2D(TextureTypeFlagBits::STORAGE | TextureTypeFlagBits::SAMPLED, extent, 1U, 6U);
			bd.bloomMipTextures = bd.bloomTexture.createMipLevelTextures();

			bd.bufferTexture = DynamicTexture2D(TextureTypeFlagBits::STORAGE | TextureTypeFlagBits::SAMPLED, extent, 1U, bd.bloomMipTextures.size() - 1);
			bd.bufferMipTextures = bd.bufferTexture.createMipLevelTextures();

			//bd.outputTexture = DynamicTexture2D(TextureTypeFlagBits::STORAGE | TextureTypeFlagBits::SAMPLED, tex.getExtent(), sa::FormatPrecisionFlagBits::e8Bit, sa::FormatDimensionFlagBits::e4, sa::FormatTypeFlagBits::UNORM);
			bd.outputTexture = DynamicTexture2D(TextureTypeFlagBits::STORAGE | TextureTypeFlagBits::SAMPLED, tex.getExtent());

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
		}
		

		uint32_t threadX = std::ceil(extent.width / 32.f);
		uint32_t threadY = std::ceil(extent.height / 32.f);

		// Filter
		context.updateDescriptorSet(bd.filterDescriptorSet, 0, tex, m_sampler);
		context.updateDescriptorSet(bd.filterDescriptorSet, 1, (Texture)bd.bloomMipTextures[0]);
		
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
	
			context.updateDescriptorSet(bd.blurDescriptorSets[i], 0, (Texture)bd.bloomMipTextures[i]);
			context.updateDescriptorSet(bd.blurDescriptorSets[i], 1, (Texture)bd.bloomMipTextures[i + 1]);

			context.bindDescriptorSet(bd.blurDescriptorSets[i], m_blurComputePipeline);
			context.dispatch(threadX, threadY, 1);
		}

		// Upsample + combine

		Texture2D smallImage = bd.bloomMipTextures[bd.bloomMipTextures.size() - 1];
		Texture2D bigImage = bd.bloomMipTextures[bd.bloomMipTextures.size() - 2];
		context.bindPipeline(m_upsamplePipeline);
		for (int i = (int)bd.bufferMipTextures.size() - 1; i >= 0; i--) {
			threadX = threadX << 1;
			threadY = threadY << 1;
			
			context.updateDescriptorSet(bd.upsampleDescriptorSets[i], 0, smallImage);
			context.updateDescriptorSet(bd.upsampleDescriptorSets[i], 1, bigImage);
			context.updateDescriptorSet(bd.upsampleDescriptorSets[i], 2, (Texture)bd.bufferMipTextures[i]);

			context.bindDescriptorSet(bd.upsampleDescriptorSets[i], m_upsamplePipeline);
			context.dispatch(threadX, threadY, 1);
			
			if (i > 0) {
				smallImage = bd.bufferMipTextures[i];
				bigImage = bd.bloomMipTextures[i - 1];
			}

		}

		threadX = threadX << 1;
		threadY = threadY << 1;

		// Composite + Tonemap
		context.bindPipeline(m_compositePipeline);
		context.updateDescriptorSet(bd.compositeDescriptorSet, 0, tex, m_sampler);
		context.updateDescriptorSet(bd.compositeDescriptorSet, 1, (Texture)bd.bufferMipTextures[0]);
		context.updateDescriptorSet(bd.compositeDescriptorSet, 2, (Texture)bd.bufferMipTextures[0]);

		context.updateDescriptorSet(bd.compositeDescriptorSet, 3, (Texture)bd.outputTexture);

		context.bindDescriptorSet(bd.compositeDescriptorSet, m_compositePipeline);
		context.dispatch(threadX, threadY, 1);

		m_pRenderTechnique->drawData.finalTexture = bd.outputTexture;
		pRenderTarget->outputTexture = bd.outputTexture;

		for (auto& image : bd.bloomMipTextures) {
			image.swap();
		}
		for (auto& image : bd.bufferMipTextures) {
			image.swap();
		}
		bd.bloomTexture.swap();
		bd.bufferTexture.swap();
		bd.outputTexture.swap();

	}

	const Texture2D& BloomRenderLayer::getOutputTexture() const {
		return {};
	}
}
