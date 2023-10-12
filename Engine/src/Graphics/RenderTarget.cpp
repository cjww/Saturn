#include "pch.h"
#include "RenderTarget.h"
#include "IRenderTechnique.h"

#include "Engine.h"

namespace sa {

	void RenderTarget::initializeBloomData(RenderContext& context, Extent extent, const DynamicTexture* colorTexture, ShaderSet& bloomShader, ResourceID sampler) {
		//Textures
		m_bloomData.bloomTexture = DynamicTexture2D(TextureTypeFlagBits::STORAGE | TextureTypeFlagBits::SAMPLED, extent, 1U, 6U);
		m_bloomData.bloomMipTextures = m_bloomData.bloomTexture.createMipLevelTextures();

		m_bloomData.bufferTexture = DynamicTexture2D(TextureTypeFlagBits::STORAGE | TextureTypeFlagBits::SAMPLED, extent, 1U, m_bloomData.bloomMipTextures.size() - 1);
		m_bloomData.bufferMipTextures = m_bloomData.bufferTexture.createMipLevelTextures();

		//bloomData.outputTexture = DynamicTexture2D(TextureTypeFlagBits::STORAGE | TextureTypeFlagBits::SAMPLED, tex.getExtent(), sa::FormatPrecisionFlagBits::e8Bit, sa::FormatDimensionFlagBits::e4, sa::FormatTypeFlagBits::UNORM);
		m_bloomData.outputTexture = DynamicTexture2D(TextureTypeFlagBits::STORAGE | TextureTypeFlagBits::SAMPLED, colorTexture->getExtent());

		// DescriptorSets
		if (m_bloomData.filterDescriptorSet == NULL_RESOURCE)
			m_bloomData.filterDescriptorSet = bloomShader.allocateDescriptorSet(0);

		if (m_bloomData.blurDescriptorSets.empty()) {
			m_bloomData.blurDescriptorSets.resize(m_bloomData.bloomMipTextures.size() - 1);
			for (size_t i = 0; i < m_bloomData.blurDescriptorSets.size(); i++) {
				m_bloomData.blurDescriptorSets[i] = bloomShader.allocateDescriptorSet(0);
			}
		}

		if (m_bloomData.upsampleDescriptorSets.empty()) {
			m_bloomData.upsampleDescriptorSets.resize(m_bloomData.bloomMipTextures.size() - 1);
			for (size_t i = 0; i < m_bloomData.upsampleDescriptorSets.size(); i++) {
				m_bloomData.upsampleDescriptorSets[i] = bloomShader.allocateDescriptorSet(0);
			}
		}

		if (m_bloomData.compositeDescriptorSet == NULL_RESOURCE)
			m_bloomData.compositeDescriptorSet = bloomShader.allocateDescriptorSet(0);



		for (int i = 0; i < m_bloomData.bloomTexture.getTextureCount(); i++) {
			context.transitionTexture(m_bloomData.bloomTexture.getTexture(i), sa::Transition::NONE, sa::Transition::COMPUTE_SHADER_WRITE);
			context.transitionTexture(m_bloomData.bufferTexture.getTexture(i), sa::Transition::NONE, sa::Transition::COMPUTE_SHADER_WRITE);
			context.transitionTexture(m_bloomData.outputTexture.getTexture(i), sa::Transition::NONE, sa::Transition::COMPUTE_SHADER_WRITE);
		}

		m_renderer.updateDescriptorSet(m_bloomData.filterDescriptorSet, 0, *colorTexture, sampler);
		m_renderer.updateDescriptorSet(m_bloomData.filterDescriptorSet, 1, m_bloomData.bloomMipTextures[0]);
		m_renderer.updateDescriptorSet(m_bloomData.filterDescriptorSet, 2, m_bloomData.bloomMipTextures[0]);
		m_renderer.updateDescriptorSet(m_bloomData.filterDescriptorSet, 3, m_bloomData.bloomMipTextures[0]);
		for (size_t i = 0; i < m_bloomData.bloomMipTextures.size() - 1; i++) {
			m_renderer.updateDescriptorSet(m_bloomData.blurDescriptorSets[i], 0, m_bloomData.bloomMipTextures[i], sampler);
			m_renderer.updateDescriptorSet(m_bloomData.blurDescriptorSets[i], 1, m_bloomData.bloomMipTextures[i]);
			m_renderer.updateDescriptorSet(m_bloomData.blurDescriptorSets[i], 2, m_bloomData.bloomMipTextures[i]);
			m_renderer.updateDescriptorSet(m_bloomData.blurDescriptorSets[i], 3, m_bloomData.bloomMipTextures[i + 1]);
		}

		DynamicTexture2D smallImage = m_bloomData.bloomMipTextures[m_bloomData.bloomMipTextures.size() - 1];
		DynamicTexture2D bigImage = m_bloomData.bloomMipTextures[m_bloomData.bloomMipTextures.size() - 2];
		for (int i = (int)m_bloomData.bufferMipTextures.size() - 1; i >= 0; i--) {
			m_renderer.updateDescriptorSet(m_bloomData.upsampleDescriptorSets[i], 0, *colorTexture, sampler);
			m_renderer.updateDescriptorSet(m_bloomData.upsampleDescriptorSets[i], 1, smallImage);
			m_renderer.updateDescriptorSet(m_bloomData.upsampleDescriptorSets[i], 2, bigImage);
			m_renderer.updateDescriptorSet(m_bloomData.upsampleDescriptorSets[i], 3, m_bloomData.bufferMipTextures[i]);
			if (i > 0) {
				smallImage = m_bloomData.bufferMipTextures[i];
				bigImage = m_bloomData.bloomMipTextures[i - 1];
			}
		}

		m_renderer.updateDescriptorSet(m_bloomData.compositeDescriptorSet, 0, *colorTexture, sampler);
		m_renderer.updateDescriptorSet(m_bloomData.compositeDescriptorSet, 1, m_bloomData.bufferMipTextures[0]);
		m_renderer.updateDescriptorSet(m_bloomData.compositeDescriptorSet, 2, m_bloomData.bufferMipTextures[0]);
		m_renderer.updateDescriptorSet(m_bloomData.compositeDescriptorSet, 3, m_bloomData.outputTexture);
	
		m_bloomData.isInitialized = true;
	}

	void RenderTarget::cleanupBloomData() {
		if (m_bloomData.bloomTexture.isValid()) {
			for (auto& tex : m_bloomData.bloomMipTextures) {
				tex.destroy();
			}
			m_bloomData.bloomTexture.destroy();
		}
		if (m_bloomData.bufferTexture.isValid()) {
			for (auto& tex : m_bloomData.bufferMipTextures) {
				tex.destroy();
			}
			m_bloomData.bufferTexture.destroy();
		}
		if (m_bloomData.outputTexture.isValid())
			m_bloomData.outputTexture.destroy();
	}

	void RenderTarget::initializeMainRenderData(ResourceID colorRenderProgram, ResourceID depthPreRenderProgram, 
		ShaderSet& lightCullingShader,
		ShaderSet& debugHeatmapShader,
		ResourceID sampler, Extent extent) 
	{
		MainRenderData& data = m_mainRenderData;

		Format colorFormat = m_renderer.getAttachmentFormat(colorRenderProgram, 0);
		Format depthFormat = m_renderer.getAttachmentFormat(colorRenderProgram, 1);


		data.colorTexture = DynamicTexture2D(TextureTypeFlagBits::COLOR_ATTACHMENT | TextureTypeFlagBits::SAMPLED, extent, colorFormat);
		data.depthTexture = DynamicTexture2D(TextureTypeFlagBits::DEPTH_ATTACHMENT | TextureTypeFlagBits::SAMPLED, extent, depthFormat);


		//Depth pre pass
		data.depthFramebuffer = m_renderer.createFramebuffer(depthPreRenderProgram, { (DynamicTexture)data.depthTexture });

		// Light culling pass
		data.tileCount = { extent.width, extent.height };
		data.tileCount += (TILE_SIZE - data.tileCount % TILE_SIZE);
		data.tileCount /= TILE_SIZE;

		size_t totalTileCount = data.tileCount.x * data.tileCount.y;
		if (data.lightCullingDescriptorSet == NULL_RESOURCE)
			data.lightCullingDescriptorSet = lightCullingShader.allocateDescriptorSet(0);

		data.lightIndexBuffer = m_renderer.createDynamicBuffer(BufferType::STORAGE, sizeof(uint32_t) * MAX_LIGHTS_PER_TILE * totalTileCount);

		// Color pass
		data.colorFramebuffer = m_renderer.createFramebuffer(colorRenderProgram, { (DynamicTexture)data.colorTexture, data.depthTexture });
		

		m_renderer.updateDescriptorSet(data.lightCullingDescriptorSet, 0, data.depthTexture, sampler);	// read depth texture
		m_renderer.updateDescriptorSet(data.lightCullingDescriptorSet, 1, data.lightIndexBuffer);		// write what lights are in what tiles

		// ----------- DEBUG -------------------
		data.debugLightHeatmap = DynamicTexture2D(TextureTypeFlagBits::COLOR_ATTACHMENT | TextureTypeFlagBits::SAMPLED, { data.tileCount.x, data.tileCount.y });
		data.debugLightHeatmapRenderProgram = m_renderer.createRenderProgram()
			.addColorAttachment(AttachmentFlagBits::eClear | AttachmentFlagBits::eSampled | AttachmentFlagBits::eStore, data.debugLightHeatmap)
			.beginSubpass()
			.addAttachmentReference(0, SubpassAttachmentUsage::ColorTarget)
			.endSubpass()
			.end();


		data.debugLightHeatmapFramebuffer = m_renderer.createFramebuffer(data.debugLightHeatmapRenderProgram, { data.debugLightHeatmap });
		data.debugLightHeatmapPipeline = m_renderer.createGraphicsPipeline(
			data.debugLightHeatmapRenderProgram,
			0,
			{ data.tileCount.x, data.tileCount.y },
			debugHeatmapShader);

		data.debugLightHeatmapDescriptorSet = debugHeatmapShader.allocateDescriptorSet(0);
		m_renderer.updateDescriptorSet(data.debugLightHeatmapDescriptorSet, 0, data.lightIndexBuffer);
		// ----------------------------------

		data.isInitialized = true;

	}

	void RenderTarget::cleanupMainRenderData() {
		if (m_mainRenderData.colorTexture.isValid())
			m_mainRenderData.colorTexture.destroy();
		if (m_mainRenderData.depthTexture.isValid())
			m_mainRenderData.depthTexture.destroy();

		if (m_mainRenderData.depthFramebuffer != NULL_RESOURCE) {
			m_renderer.destroyFramebuffer(m_mainRenderData.depthFramebuffer);
			m_mainRenderData.depthFramebuffer = NULL_RESOURCE;
		}

		if (m_mainRenderData.lightIndexBuffer.isValid())
			m_mainRenderData.lightIndexBuffer.destroy();

		if (m_mainRenderData.colorFramebuffer != NULL_RESOURCE) {
			m_renderer.destroyFramebuffer(m_mainRenderData.colorFramebuffer);
			m_mainRenderData.colorFramebuffer = NULL_RESOURCE;
		}

		//DEBUG 
		if (m_mainRenderData.debugLightHeatmap.isValid())
			m_mainRenderData.debugLightHeatmap.destroy();
		
		if (m_mainRenderData.debugLightHeatmapFramebuffer != NULL_RESOURCE) {
			m_renderer.destroyFramebuffer(m_mainRenderData.debugLightHeatmapFramebuffer);
			m_mainRenderData.debugLightHeatmapFramebuffer = NULL_RESOURCE;
		}

		if (m_mainRenderData.debugLightHeatmapPipeline != NULL_RESOURCE) {
			m_renderer.destroyPipeline(m_mainRenderData.debugLightHeatmapPipeline);
			m_mainRenderData.debugLightHeatmapPipeline = NULL_RESOURCE;
		}

		if(m_mainRenderData.debugLightHeatmapRenderProgram != NULL_RESOURCE) {
			m_renderer.destroyRenderProgram(m_mainRenderData.debugLightHeatmapRenderProgram);
			m_mainRenderData.debugLightHeatmapRenderProgram = NULL_RESOURCE;
		}



	}

	void RenderTarget::setOutputTexture(const DynamicTexture& dynamicTexture) {
		m_pOutputTexture = (DynamicTexture*)&dynamicTexture;
	}

	RenderTarget::RenderTarget(const AssetHeader& header) 
		: IAsset(header)
		, m_renderer(Renderer::get())
		, m_isActive(true)
		, m_extent({256, 256})
		, m_pOutputTexture(nullptr)
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
		m_extent = extent;
		
		m_mainRenderData.isInitialized = false; // initialize main data in main render pass
		m_bloomData.isInitialized = false; // initialize bloom data in bloom pass
	}
	
	void RenderTarget::initialize(Engine* pEngine, RenderWindow* pWindow) {
		pEngine->on<engine_event::WindowResized>([this](engine_event::WindowResized& e, Engine& engine) {
			this->resize(e.newExtent);
			m_extent = e.newExtent;
		});
		
		initialize(pWindow->getCurrentExtent());
	}

	void RenderTarget::destroy() {
		cleanupMainRenderData();
		cleanupBloomData();
	}

	void RenderTarget::resize(Extent extent) {
		m_extent = extent;

		m_mainRenderData.isInitialized = false; // initialize main data in main render pass
		m_bloomData.isInitialized = false; // initialize bloom data in bloom pass

		m_pOutputTexture = nullptr;
	}

	void RenderTarget::swap() {
		if (m_mainRenderData.isInitialized) {
			m_mainRenderData.colorTexture.swap();
			m_mainRenderData.depthTexture.swap();
			m_mainRenderData.lightIndexBuffer.swap();

			m_renderer.swapFramebuffer(m_mainRenderData.colorFramebuffer);
			m_renderer.swapFramebuffer(m_mainRenderData.depthFramebuffer);

			m_mainRenderData.debugLightHeatmap.swap();
			m_renderer.swapFramebuffer(m_mainRenderData.debugLightHeatmapFramebuffer);

		}
		if (m_bloomData.isInitialized) {
			m_bloomData.outputTexture.swap();
		}

	}

	bool RenderTarget::isReady() const {
		return
			m_pOutputTexture != nullptr &&
			m_pOutputTexture->isValid() &&
			m_mainRenderData.isInitialized;
	}

	const RenderTarget::MainRenderData& RenderTarget::getMainRenderData() const {
		return m_mainRenderData;
	}

	const RenderTarget::BloomData& RenderTarget::getBloomData() const {
		return m_bloomData;
	}

	void RenderTarget::setRenderDebugHeatmap(bool value) {
		m_mainRenderData.renderDebugHeatmap = value;
	}

	const Extent& RenderTarget::getExtent() const {
		return m_extent;
	}

	const Texture& RenderTarget::getOutputTexture() const {
		if (m_pOutputTexture)
			return m_pOutputTexture->getTexture();
		return *sa::AssetManager::get().loadDefaultBlackTexture();
	}

	void RenderTarget::setActive(bool isActive) {
		m_isActive = isActive;
	}

	bool RenderTarget::isActive() const {
		return m_isActive;
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