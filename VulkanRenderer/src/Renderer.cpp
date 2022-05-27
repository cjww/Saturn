#include "pch.h"
#include "Renderer.hpp"

#include "VulkanCore.hpp"
#include "debugFunctions.h"


#include "Resources/Swapchain.hpp"
#include "Resources\RenderProgram.hpp"

namespace sa {


	Renderer& Renderer::get() {
		static Renderer instance;
		return instance;
	}

	Renderer::Renderer() {
		try {
			vk::ApplicationInfo info = {
				.pApplicationName = "Saturn App",
				.applicationVersion = 0,
				.pEngineName = "Saturn",
				.engineVersion = 0,
				.apiVersion = VK_API_VERSION_1_3
			}; 

			m_pCore = std::make_unique<VulkanCore>();
			
			m_pCore->init(info);

			ResourceManager::get().setCleanupFunction<Swapchain>([](Swapchain* p) { p->destroy(); });
			ResourceManager::get().setCleanupFunction<FramebufferSet>([](FramebufferSet* p) { p->destroy(); });
			ResourceManager::get().setCleanupFunction<RenderProgram>([](RenderProgram* p) { p->destroy(); });
			ResourceManager::get().setCleanupFunction<Pipeline>([](Pipeline* p) { p->destroy(); });
			ResourceManager::get().setCleanupFunction<DescriptorSet>([](DescriptorSet* p) { p->destroy(); });
			ResourceManager::get().setCleanupFunction<vk::Sampler>([&](vk::Sampler* p) { m_pCore->getDevice().destroySampler(*p); });

		}
		catch (const std::exception& e) {
			DEBUG_LOG_ERROR(e.what());
		}
	}
	Renderer::~Renderer() {
		m_pCore->getDevice().waitIdle();

		ResourceManager::get().clearContainer<vk::Sampler>();
		ResourceManager::get().clearContainer<DescriptorSet>();
		ResourceManager::get().clearContainer<Pipeline>();
		ResourceManager::get().clearContainer<FramebufferSet>();
		ResourceManager::get().clearContainer<RenderProgram>();
		ResourceManager::get().clearContainer<Swapchain>();

		m_pCore->cleanup();
	}

	ResourceID Renderer::createSwapchain(GLFWwindow* pWindow) {
		ResourceID id = ResourceManager::get().insert<Swapchain>();
		Swapchain* pSwapchain = ResourceManager::get().get<Swapchain>(id);
		pSwapchain->create(m_pCore.get(), pWindow);
		return id;
	}

	void Renderer::destroySwapchain(ResourceID id) {
		ResourceManager::get().remove<Swapchain>(id);
	}

	uint32_t Renderer::getSwapchainImageCount(ResourceID swapchain) {
		Swapchain* pSwapchain = RenderContext::getSwapchain(swapchain);
		return pSwapchain->getImageCount();
	}

	RenderProgramFactory Renderer::createRenderProgram() {
		return RenderProgramFactory(m_pCore.get());
	}

	void Renderer::setClearColor(ResourceID renderProgram, Color color, uint32_t attachmentIndex) {
		RenderProgram* pRenderProgram = RenderContext::getRenderProgram(renderProgram);
		pRenderProgram->setClearColor(attachmentIndex, color);
	}

	void Renderer::setClearColor(ResourceID renderProgram, Color color) {
		RenderProgram* pRenderProgram = RenderContext::getRenderProgram(renderProgram);
		pRenderProgram->setClearColor(color);
	}

	ResourceID Renderer::createFramebuffer(ResourceID renderProgram, const std::vector<Texture2D>& attachmentTextures, uint32_t count, uint32_t layers) {
		if (attachmentTextures.empty())
			throw std::runtime_error("At least one attachmnet is required to create a framebuffer");

		RenderProgram* pRenderProgram = RenderContext::getRenderProgram(renderProgram);

		std::vector<std::vector<vk::ImageView>> framebufferViews(count);
		Extent extent = attachmentTextures[0].getExtent();
		for (uint32_t i = 0; i < count; i++) {
			for (auto& texture : attachmentTextures) {
				framebufferViews[i].push_back(*texture.getView());
				if (extent.width != texture.getExtent().width || extent.height != texture.getExtent().height) {
					throw std::runtime_error("All attachments must be of the same size");
				}
			}
		}

		return ResourceManager::get().insert<FramebufferSet>(
			m_pCore->createFrameBufferSet(pRenderProgram->getRenderPass(), framebufferViews, extent.width, extent.height, layers));
	}

	ResourceID Renderer::createSwapchainFramebuffer(ResourceID renderProgram, ResourceID swapchain, const std::vector<Texture2D>& additionalAttachmentTextures, uint32_t layers) {
		Swapchain* pSwapchain = RenderContext::getSwapchain(swapchain);
		RenderProgram* pRenderProgram = RenderContext::getRenderProgram(renderProgram);

		std::vector<vk::ImageView> swapchainViews = pSwapchain->getImageViews();
		uint32_t count = static_cast<uint32_t>(swapchainViews.size());
		std::vector<std::vector<vk::ImageView>> framebufferViews(count);
		Extent extent = pSwapchain->getExtent();
		for (uint32_t i = 0; i < count; i++) {
			framebufferViews[i].resize(additionalAttachmentTextures.size() + 1);
			framebufferViews[i][0] = swapchainViews[i];
			for (uint32_t j = 1; j < (uint32_t)framebufferViews[i].size(); j++) {
				framebufferViews[i][j] = *additionalAttachmentTextures[j - 1].getView();
				if (extent.width != additionalAttachmentTextures[j - 1].getExtent().width || extent.height != additionalAttachmentTextures[j - 1].getExtent().height) {
					throw std::runtime_error("All attachments must be of the same size");
				}
			}
		}

		return ResourceManager::get().insert<FramebufferSet>(m_pCore->createFrameBufferSet(pRenderProgram->getRenderPass(), framebufferViews, extent.width, extent.height, layers));
	}

	void Renderer::destroyFramebuffer(ResourceID framebuffer) {
		ResourceManager::get().remove<FramebufferSet>(framebuffer);
	}

	ResourceID Renderer::createGraphicsPipeline(ResourceID renderProgram, uint32_t subpassIndex, Extent extent, const std::string& vertexShader, const std::string& fragmentShader) {
		RenderProgram* pRenderProgram = RenderContext::getRenderProgram(renderProgram);
		Shader vShader(m_pCore->getDevice(), vertexShader.c_str(), vk::ShaderStageFlagBits::eVertex);
		Shader fShader(m_pCore->getDevice(), fragmentShader.c_str(), vk::ShaderStageFlagBits::eFragment);
		ShaderSet set(m_pCore->getDevice(), vShader, fShader);

		return pRenderProgram->createPipeline(set, subpassIndex, extent);
	}
	
	ResourceID Renderer::createGraphicsPipeline(ResourceID renderProgram, uint32_t subpassIndex, Extent extent, const std::string& vertexShader, const std::string& geometryShader, const std::string& fragmentShader) {
		RenderProgram* pRenderProgram = RenderContext::getRenderProgram(renderProgram);
		Shader vShader(m_pCore->getDevice(), vertexShader.c_str(), vk::ShaderStageFlagBits::eVertex);
		Shader gShader(m_pCore->getDevice(), geometryShader.c_str(), vk::ShaderStageFlagBits::eGeometry);
		Shader fShader(m_pCore->getDevice(), fragmentShader.c_str(), vk::ShaderStageFlagBits::eFragment);
		ShaderSet set(m_pCore->getDevice(), vShader, gShader, fShader);

		return pRenderProgram->createPipeline(set, subpassIndex, extent);
	}

	void Renderer::destroyPipeline(ResourceID pipeline) {
		ResourceManager::get().remove<Pipeline>(pipeline);
	}

	ResourceID Renderer::allocateDescriptorSet(ResourceID pipeline, uint32_t setIndex, uint32_t backBufferCount) {
		Pipeline* pPipeline = RenderContext::getPipeline(pipeline);
		return ResourceManager::get().insert<DescriptorSet>(pPipeline->allocateDescriptSet(setIndex, backBufferCount));
	}

	void Renderer::updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const Buffer& buffer) {
		DescriptorSet* pDescriptorSet = RenderContext::getDescriptorSet(descriptorSet);
		const DeviceBuffer* pDeviceBuffer = (const DeviceBuffer*)buffer;
		pDescriptorSet->update(binding, pDeviceBuffer->buffer, pDeviceBuffer->size, 0, UINT32_MAX);
	}

	void Renderer::updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const Texture2D& texture, ResourceID sampler) {
		DescriptorSet* pDescriptorSet = RenderContext::getDescriptorSet(descriptorSet);
		vk::Sampler* pSampler = RenderContext::getSampler(sampler);
		pDescriptorSet->update(binding, vk::ImageLayout::eShaderReadOnlyOptimal, *texture.getView(), pSampler, UINT32_MAX);
	}

	void Renderer::updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const Texture2D& texture) {
		DescriptorSet* pDescriptorSet = RenderContext::getDescriptorSet(descriptorSet);
		pDescriptorSet->update(binding, vk::ImageLayout::eShaderReadOnlyOptimal, *texture.getView(), nullptr, UINT32_MAX);
	}

	void Renderer::freeDescriptorSet(ResourceID descriptorSet) {
		ResourceManager::get().remove<DescriptorSet>(descriptorSet);
	}

	Buffer Renderer::createBuffer(BufferType type, size_t size, void* initialData) {
		return Buffer(m_pCore.get(), type, size, initialData);
	}

	Texture2D Renderer::createTexture2D(TextureTypeFlags type, Extent extent) {
		return Texture2D(m_pCore.get(), type, extent);
	}

	Texture2D Renderer::createTexture2D(TextureTypeFlags type, Extent extent, FormatPrecisionFlags formatPrecision, FormatDimensionFlags formatDimensions, FormatTypeFlags formatType) {
		return Texture2D(m_pCore.get(), type, extent, formatPrecision, formatDimensions, formatType);
	}

	Texture2D Renderer::createTexture2D(const Image& image) {
		return Texture2D(m_pCore.get(), image);
	}

	void Renderer::queueTransfer(const DataTransfer& transfer) {
		m_transferQueue.push(transfer);
	}

	ResourceID Renderer::createSampler() {
		return ResourceManager::get().insert(m_pCore->createSampler());
	}

	RenderContext Renderer::beginFrame(ResourceID swapchain) {
		Swapchain* pSwapchain = RenderContext::getSwapchain(swapchain);

		CommandBufferSet* pCommandBufferSet = pSwapchain->beginFrame();
		if (!pCommandBufferSet) {
			return {};
		}

		while (!m_transferQueue.empty()) {
			DataTransfer& transfer = m_transferQueue.front();

			switch (transfer.type) {
			case DataTransfer::Type::BUFFER_TO_IMAGE:
				m_pCore->transferBufferToColorImage(
					pCommandBufferSet->getBuffer(),
					transfer.srcBuffer->buffer,
					transfer.dstImage->image,
					transfer.dstImage->extent,
					transfer.dstImage->layout,
					vk::ImageLayout::eShaderReadOnlyOptimal,
					vk::AccessFlagBits::eShaderRead,
					vk::PipelineStageFlagBits::eFragmentShader);
				transfer.dstImage->layout = vk::ImageLayout::eShaderReadOnlyOptimal;
				break;
			case DataTransfer::Type::BUFFER_TO_BUFFER:
			case DataTransfer::Type::IMAGE_TO_BUFFER:	
			case DataTransfer::Type::IMAGE_TO_IMAGE:
				throw std::runtime_error("unimplemented case");
				break;
			default:
				throw std::runtime_error("Invalid transfer type");
				break;
			}

			m_transferQueue.pop();
		}

		return RenderContext(pCommandBufferSet);
	}

	void Renderer::endFrame(ResourceID swapchain) {
		Swapchain* pSwapchain = RenderContext::getSwapchain(swapchain);
		pSwapchain->endFrame(m_pCore->getGraphicsQueue());
	}


}

