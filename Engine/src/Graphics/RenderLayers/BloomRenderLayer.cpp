#include "pch.h"
#include "BloomRenderLayer.h"
#include "Engine.h"
namespace sa {

	void BloomRenderLayer::init() {

		m_bloomPipeline = m_renderer.createComputePipeline((Engine::getShaderDirectory() / "BloomShader.comp.spv").generic_string());


		m_bloomPreferencesDescriptorSet = m_renderer.allocateDescriptorSet(m_bloomPipeline, 1);

		m_bloomPreferencesBuffer = m_renderer.createBuffer(BufferType::UNIFORM, sizeof(BloomPreferences), &m_bloomPreferences);
		m_renderer.updateDescriptorSet(m_bloomPreferencesDescriptorSet, 0, m_bloomPreferencesBuffer);


		SamplerInfo samplerInfo = {};
		samplerInfo.minFilter = FilterMode::NEAREST;
		samplerInfo.magFilter = FilterMode::NEAREST;
		
		samplerInfo.addressModeU = SamplerAddressMode::CLAMP_TO_EDGE;
		samplerInfo.addressModeV = SamplerAddressMode::CLAMP_TO_EDGE;
		samplerInfo.addressModeW = SamplerAddressMode::CLAMP_TO_EDGE;

		m_sampler = m_renderer.createSampler(samplerInfo);

		m_stackSize = 0;
	}

	void BloomRenderLayer::cleanup() {

	}


	const Texture& BloomRenderLayer::render(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sceneCollection) {
		SA_PROFILE_FUNCTION();

		DynamicTexture* tex = &pRenderTarget->mainRenderData.colorTexture; //m_renderer.getFramebufferDynamicTexturePtr(pRenderTarget->framebuffer, 0);
		Extent extent = { std::ceil(tex->getExtent().width * 0.5f), std::ceil(tex->getExtent().height * 0.5f) };
		
		RenderTarget::BloomData& bd = pRenderTarget->bloomData;
		
		if (!bd.isInitialized) {
		/*
			// Free old data
			cleanupBloomData(bd);
			
			// Initialize
			initializeBloomData(context, extent, tex, bd);
		*/
			pRenderTarget->cleanupBloomData();
			pRenderTarget->initializeBloomData(context, extent, tex, m_bloomPipeline, m_sampler);
			bd.isInitialized = true;

		}
		

		uint32_t threadX = std::ceil(extent.width / 32.f);
		uint32_t threadY = std::ceil(extent.height / 32.f);
		m_threadCountStack[m_stackSize++] = { threadX << 1, threadY << 1 };
		m_threadCountStack[m_stackSize++] = { threadX, threadY };

		// Filter
		context.bindPipeline(m_bloomPipeline);
		context.bindDescriptorSet(m_bloomPreferencesDescriptorSet, m_bloomPipeline);

		context.bindDescriptorSet(bd.filterDescriptorSet, m_bloomPipeline);
		context.pushConstant(m_bloomPipeline, ShaderStageFlagBits::COMPUTE, 0);
		context.dispatch(threadX, threadY, 1);

		
		// Downsample + blur
		for (size_t i = 0; i < bd.bloomMipTextures.size() - 1; i++) {
			threadX += threadX % 2;
			threadX = threadX >> 1;

			threadY += threadY % 2;
			threadY = threadY >> 1;
			m_threadCountStack[m_stackSize++] = { threadX, threadY };

			context.bindDescriptorSet(bd.blurDescriptorSets[i], m_bloomPipeline);
			context.pushConstant(m_bloomPipeline, ShaderStageFlagBits::COMPUTE, 1);
			context.dispatch(threadX, threadY, 1);
		}
		// Upsample + combine

		m_stackSize--;
		for (int i = (int)bd.bufferMipTextures.size() - 1; i >= 0; i--) {
			m_stackSize--;
			threadX = m_threadCountStack[m_stackSize].width;
			threadY = m_threadCountStack[m_stackSize].height;

			context.bindDescriptorSet(bd.upsampleDescriptorSets[i], m_bloomPipeline);
			context.pushConstant(m_bloomPipeline, ShaderStageFlagBits::COMPUTE, 2);
			context.dispatch(threadX, threadY, 1);
			
		}

		threadX = m_threadCountStack[--m_stackSize].width;
		threadY = m_threadCountStack[m_stackSize].height;

		// Composite + Tonemap
		
		context.bindDescriptorSet(bd.compositeDescriptorSet, m_bloomPipeline);
		context.pushConstant(m_bloomPipeline, ShaderStageFlagBits::COMPUTE, 3);
		context.dispatch(threadX, threadY, 1);

		pRenderTarget->outputTexture = &bd.outputTexture;

		return bd.outputTexture.getTexture();
	}

	const BloomPreferences& BloomRenderLayer::getBloomPreferences() const {
		return m_bloomPreferences;
	}

	void BloomRenderLayer::setBloomPreferences(const BloomPreferences& bloomPreferences) {
		m_bloomPreferences = bloomPreferences;
		m_bloomPreferencesBuffer.write(m_bloomPreferences);
		m_renderer.updateDescriptorSet(m_bloomPreferencesDescriptorSet, 0, m_bloomPreferencesBuffer);
	}
}
