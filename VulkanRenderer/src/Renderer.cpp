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


		}
		catch (const std::exception& e) {
			DEBUG_LOG_ERROR(e.what());
		}
	}
	Renderer::~Renderer() {
		m_pCore->getDevice().waitIdle();

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

	ResourceID Renderer::createFramebuffer(ResourceID renderProgram, const std::vector<Texture2D>& attachmentTextures, uint32_t layers) {
		if (attachmentTextures.empty())
			throw std::runtime_error("At least one attachmnet is required to create a framebuffer");

		RenderProgram* pRenderProgram = RenderContext::getRenderProgram(renderProgram);

		std::vector<vk::ImageView> framebufferViews;
		Extent extent = attachmentTextures[0].getExtent();
		for (auto& texture : attachmentTextures) {
			framebufferViews.push_back(*texture.getView());
			if (extent.width != texture.getExtent().width || extent.height != texture.getExtent().height) {
				throw std::runtime_error("All attachments must be of the same size");
			}
		}


		return ResourceManager::get().insert<FramebufferSet>(
			m_pCore->createFrameBufferSet(pRenderProgram->getRenderPass(), { framebufferViews }, extent.width, extent.height, layers)
			
			);
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
			for (uint32_t j = 1; j < (uint32_t)additionalAttachmentTextures.size(); j++) {
				framebufferViews[i][j] = *additionalAttachmentTextures[j].getView();
				if (extent.width != additionalAttachmentTextures[j].getExtent().width || extent.height != additionalAttachmentTextures[j].getExtent().height) {
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
		DescriptorSet* pDedscriptorSet = RenderContext::getDescriptorSet(descriptorSet);
		const DeviceBuffer* pDeviceBuffer = (const DeviceBuffer*)buffer;
		pDedscriptorSet->update(binding, pDeviceBuffer->buffer, pDeviceBuffer->size, 0, UINT32_MAX);
	}

	void Renderer::freeDescriptorSet(ResourceID descriptorSet) {
		ResourceManager::get().remove<DescriptorSet>(descriptorSet);
	}

	Buffer Renderer::createBuffer(BufferType type, size_t size, void* initialData) {
		return Buffer(m_pCore.get(), type, size, initialData);
	}

	RenderContext Renderer::beginFrame(ResourceID swapchain) {
		Swapchain* pSwapchain = RenderContext::getSwapchain(swapchain);

		CommandBufferSet* pCommandBufferSet = pSwapchain->beginFrame();
		if (!pCommandBufferSet) {
			return {};
		}

		return RenderContext(pCommandBufferSet);
	}

	void Renderer::endFrame(ResourceID swapchain) {
		Swapchain* pSwapchain = RenderContext::getSwapchain(swapchain);
		pSwapchain->endFrame(m_pCore->getGraphicsQueue());
	}

}

