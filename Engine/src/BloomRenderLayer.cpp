#include "pch.h"
#include "Graphics/RenderLayers/BloomRenderLayer.h"
#include "Engine.h"
namespace sa {
	void BloomRenderLayer::initializeBloomData(const UUID& renderTargetID, RenderContext& context, Extent extent, const DynamicTexture* colorTexture) {
		BloomData& data = getRenderTargetData(renderTargetID);

		//Textures
		data.bloomTexture.create2D(TextureUsageFlagBits::STORAGE | TextureUsageFlagBits::SAMPLED, extent, Format::UNDEFINED, 6U);
		data.bloomMipTextures = data.bloomTexture.createMipLevelTextures();

		data.bufferTexture.create2D(TextureUsageFlagBits::STORAGE | TextureUsageFlagBits::SAMPLED, extent, Format::UNDEFINED, data.bloomMipTextures.size() - 1);
		data.bufferMipTextures = data.bufferTexture.createMipLevelTextures();

		//bloomData.outputTexture = DynamicTexture2D(TextureTypeFlagBits::STORAGE | TextureTypeFlagBits::SAMPLED, tex.getExtent(), sa::FormatPrecisionFlagBits::e8Bit, sa::FormatDimensionFlagBits::e4, sa::FormatTypeFlagBits::UNORM);
		data.outputTexture.create2D(TextureUsageFlagBits::STORAGE | TextureUsageFlagBits::SAMPLED, colorTexture->getExtent());

		// DescriptorSets
		if (data.filterDescriptorSet == NULL_RESOURCE)
			data.filterDescriptorSet = m_pipelineLayout.allocateDescriptorSet(0);

		if (data.blurDescriptorSets.empty()) {
			data.blurDescriptorSets.resize(data.bloomMipTextures.size() - 1);
			for (size_t i = 0; i < data.blurDescriptorSets.size(); i++) {
				data.blurDescriptorSets[i] = m_pipelineLayout.allocateDescriptorSet(0);
			}
		}

		if (data.upsampleDescriptorSets.empty()) {
			data.upsampleDescriptorSets.resize(data.bloomMipTextures.size() - 1);
			for (size_t i = 0; i < data.upsampleDescriptorSets.size(); i++) {
				data.upsampleDescriptorSets[i] = m_pipelineLayout.allocateDescriptorSet(0);
			}
		}

		if (data.compositeDescriptorSet == NULL_RESOURCE)
			data.compositeDescriptorSet = m_pipelineLayout.allocateDescriptorSet(0);


		for (int i = 0; i < data.bloomTexture.getTextureCount(); i++) {
			context.transitionTexture(data.bloomTexture.getTexture(i), sa::Transition::NONE, sa::Transition::COMPUTE_SHADER_WRITE);
			context.transitionTexture(data.bufferTexture.getTexture(i), sa::Transition::NONE, sa::Transition::COMPUTE_SHADER_WRITE);
			context.transitionTexture(data.outputTexture.getTexture(i), sa::Transition::NONE, sa::Transition::COMPUTE_SHADER_WRITE);
		}

		m_renderer.updateDescriptorSet(data.filterDescriptorSet, 0, *colorTexture, m_sampler);
		m_renderer.updateDescriptorSet(data.filterDescriptorSet, 1, data.bloomMipTextures[0]);
		m_renderer.updateDescriptorSet(data.filterDescriptorSet, 2, data.bloomMipTextures[0]);
		m_renderer.updateDescriptorSet(data.filterDescriptorSet, 3, data.bloomMipTextures[0]);
		for (size_t i = 0; i < data.bloomMipTextures.size() - 1; i++) {
			m_renderer.updateDescriptorSet(data.blurDescriptorSets[i], 0, data.bloomMipTextures[i], m_sampler);
			m_renderer.updateDescriptorSet(data.blurDescriptorSets[i], 1, data.bloomMipTextures[i]);
			m_renderer.updateDescriptorSet(data.blurDescriptorSets[i], 2, data.bloomMipTextures[i]);
			m_renderer.updateDescriptorSet(data.blurDescriptorSets[i], 3, data.bloomMipTextures[i + 1]);
		}

		DynamicTexture smallImage = data.bloomMipTextures[data.bloomMipTextures.size() - 1];
		DynamicTexture bigImage = data.bloomMipTextures[data.bloomMipTextures.size() - 2];
		for (int i = (int)data.bufferMipTextures.size() - 1; i >= 0; i--) {
			m_renderer.updateDescriptorSet(data.upsampleDescriptorSets[i], 0, *colorTexture, m_sampler);
			m_renderer.updateDescriptorSet(data.upsampleDescriptorSets[i], 1, smallImage);
			m_renderer.updateDescriptorSet(data.upsampleDescriptorSets[i], 2, bigImage);
			m_renderer.updateDescriptorSet(data.upsampleDescriptorSets[i], 3, data.bufferMipTextures[i]);
			if (i > 0) {
				smallImage = data.bufferMipTextures[i];
				bigImage = data.bloomMipTextures[i - 1];
			}
		}

		m_renderer.updateDescriptorSet(data.compositeDescriptorSet, 0, *colorTexture, m_sampler);
		m_renderer.updateDescriptorSet(data.compositeDescriptorSet, 1, data.bufferMipTextures[0]);
		m_renderer.updateDescriptorSet(data.compositeDescriptorSet, 2, data.bufferMipTextures[0]);
		m_renderer.updateDescriptorSet(data.compositeDescriptorSet, 3, data.outputTexture);

		data.isInitialized = true;

		SA_DEBUG_LOG_INFO("Initialized Bloom data for RenderTarget UUID: ", renderTargetID, " with extent { w:", extent.width * 2, ", h:", extent.height * 2, " }");
	}

	void BloomRenderLayer::cleanupBloomData(const UUID& renderTargetID) {
		BloomData& data = getRenderTargetData(renderTargetID);

		if (data.bloomTexture.isValid()) {
			for (auto& tex : data.bloomMipTextures) {
				tex.destroy();
			}
			data.bloomTexture.destroy();
		}
		if (data.bufferTexture.isValid()) {
			for (auto& tex : data.bufferMipTextures) {
				tex.destroy();
			}
			data.bufferTexture.destroy();
		}
		if (data.outputTexture.isValid())
			data.outputTexture.destroy();
	}

	void BloomRenderLayer::init() {
		if (m_isInitialized)
			return;
		auto code = ReadSPVFile((Engine::GetShaderDirectory() / "BloomShader.comp.spv").generic_string().c_str());


		m_bloomShader.create(code, ShaderStageFlagBits::COMPUTE);
		m_pipelineLayout.createFromShaders({ m_bloomShader });

		m_bloomPipeline = m_renderer.createComputePipeline(m_bloomShader, m_pipelineLayout);

		m_bloomPreferencesDescriptorSet = m_pipelineLayout.allocateDescriptorSet(1);

		BloomPreferences& prefs = getPreferences();
		m_bloomPreferencesBuffer.create(BufferType::UNIFORM, sizeof(BloomPreferences), &prefs);
		m_renderer.updateDescriptorSet(m_bloomPreferencesDescriptorSet, 0, m_bloomPreferencesBuffer);


		SamplerInfo samplerInfo = {};
		samplerInfo.minFilter = FilterMode::NEAREST;
		samplerInfo.magFilter = FilterMode::NEAREST;
		
		samplerInfo.addressModeU = SamplerAddressMode::CLAMP_TO_EDGE;
		samplerInfo.addressModeV = SamplerAddressMode::CLAMP_TO_EDGE;
		samplerInfo.addressModeW = SamplerAddressMode::CLAMP_TO_EDGE;

		m_sampler = m_renderer.createSampler(samplerInfo);

		m_stackSize = 0;

		m_isInitialized = true;
	}

	void BloomRenderLayer::cleanup() {
		m_pipelineLayout.destroy();
		m_bloomShader.destroy();
		m_renderer.destroyPipeline(m_bloomPipeline);
	}

	void BloomRenderLayer::onRenderTargetResize(UUID renderTargetID, Extent oldExtent, Extent newExtent) {
		BloomData& bd = getRenderTargetData(renderTargetID);
		bd.isInitialized = false;
	}

	void BloomRenderLayer::onPreferencesUpdated() {
		BloomPreferences& prefs = getPreferences();
		m_bloomPreferencesBuffer.write(prefs);
		m_renderer.updateDescriptorSet(m_bloomPreferencesDescriptorSet, 0, m_bloomPreferencesBuffer);
	}

	bool BloomRenderLayer::render(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sceneCollection) {
		SA_PROFILE_FUNCTION();

		const DynamicTexture* tex = pRenderTarget->getOutputTextureDynamic();
		if (!tex)
			return false;
		Extent extent = {
			static_cast<uint32_t>(std::ceil(tex->getExtent().width * 0.5f)),
			static_cast<uint32_t>(std::ceil(tex->getExtent().height * 0.5f))
		};

		BloomData& bd = getRenderTargetData(pRenderTarget->getID());
		
		if (!bd.isInitialized) {
			// Free old data
			cleanupBloomData(pRenderTarget->getID());
			// Initialize
			initializeBloomData(pRenderTarget->getID(), context, extent, tex);
		}
		

		uint32_t threadX = std::ceil(extent.width / 32.f);
		uint32_t threadY = std::ceil(extent.height / 32.f);
		m_threadCountStack[m_stackSize++] = { threadX << 1, threadY << 1 };
		m_threadCountStack[m_stackSize++] = { threadX, threadY };

		// Filter
		context.bindPipelineLayout(m_pipelineLayout);
		context.bindPipeline(m_bloomPipeline);
		context.bindDescriptorSet(m_bloomPreferencesDescriptorSet);

		context.bindDescriptorSet(bd.filterDescriptorSet);
		context.pushConstant(ShaderStageFlagBits::COMPUTE, 0);
		context.dispatch(threadX, threadY, 1);

		
		// Downsample + blur
		for (size_t i = 0; i < bd.bloomMipTextures.size() - 1; i++) {
			threadX += threadX % 2;
			threadX = threadX >> 1;

			threadY += threadY % 2;
			threadY = threadY >> 1;
			m_threadCountStack[m_stackSize++] = { threadX, threadY };

			context.bindDescriptorSet(bd.blurDescriptorSets[i]);
			context.pushConstant(ShaderStageFlagBits::COMPUTE, 1);
			context.dispatch(threadX, threadY, 1);
		}
		// Upsample + combine

		m_stackSize--;
		for (int i = (int)bd.bufferMipTextures.size() - 1; i >= 0; i--) {
			m_stackSize--;
			threadX = m_threadCountStack[m_stackSize].width;
			threadY = m_threadCountStack[m_stackSize].height;

			context.bindDescriptorSet(bd.upsampleDescriptorSets[i]);
			context.pushConstant(ShaderStageFlagBits::COMPUTE, 2);
			context.dispatch(threadX, threadY, 1);

			bd.bloomMipTextures[i].sync(context);
		}

		threadX = m_threadCountStack[--m_stackSize].width;
		threadY = m_threadCountStack[m_stackSize].height;

		// Composite + Tonemap
		
		context.bindDescriptorSet(bd.compositeDescriptorSet);
		context.pushConstant(ShaderStageFlagBits::COMPUTE, 3);
		context.dispatch(threadX, threadY, 1);
		
		bd.outputTexture.sync(context);
		bd.bloomTexture.sync(context);
		bd.bufferTexture.sync(context);

		pRenderTarget->setOutputTexture(bd.outputTexture);
		Engine::GetEngineStatistics().dispatchCalls += bd.bufferMipTextures.size() * 2 + 2;
		return true;
	}

	bool BloomRenderLayer::postRender(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget,
		SceneCollection& sceneCollection)
	{
		return true;
	}

}
