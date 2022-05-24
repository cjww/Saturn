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

	Swapchain* Renderer::getSwapchain(ResourceID id) {
		Swapchain* pSwapchain = ResourceManager::get().get<Swapchain>(id);
		if (!pSwapchain)
			throw std::runtime_error("Nonexistent swapchain: " + id);
		return pSwapchain;
	}

	RenderProgram* Renderer::getRenderProgram(ResourceID id) {
		RenderProgram* pRenderProgram = ResourceManager::get().get<RenderProgram>(id);
		if (!pRenderProgram)
			throw std::runtime_error("Nonexistent render program: " + id);
		return pRenderProgram;
	}

	FramebufferSet* Renderer::getFramebufferSet(ResourceID id) {
		FramebufferSet* pFramebufferSet = ResourceManager::get().get<FramebufferSet>(id);
		if (!pFramebufferSet)
			throw std::runtime_error("Nonexistent framebuffer: " + id);
		return pFramebufferSet;
	}

	Renderer::Renderer() {
		try {
			m_pCore = std::make_unique<VulkanCore>();
			m_pCore->init();

			ResourceManager::get().setCleanupFunction<Swapchain>([](Swapchain* p) { p->destroy(); });
			ResourceManager::get().setCleanupFunction<FramebufferSet>([](FramebufferSet* p) { p->destroy(); });
			ResourceManager::get().setCleanupFunction<RenderProgram>([](RenderProgram* p) { p->destroy(); });


		}
		catch (const std::exception& e) {
			DEBUG_LOG_ERROR(e.what());
		}
	}
	Renderer::~Renderer() {
		m_pCore->getDevice().waitIdle();

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

	RenderProgramFactory Renderer::createRenderProgram() {
		return RenderProgramFactory(m_pCore.get());
	}

	ResourceID Renderer::createFramebuffer(ResourceID renderProgram, const std::vector<Texture2D>& attachmentTextures, uint32_t layers) {
		if (attachmentTextures.empty())
			throw std::runtime_error("At least one attachmnet is required to create a framebuffer");

		RenderProgram* pRenderProgram = getRenderProgram(renderProgram);

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

	ResourceID Renderer::createSwapchainFramebuffer(ResourceID swapchain, ResourceID renderProgram, const std::vector<Texture2D>& additionalAttachmentTextures, uint32_t layers) {
		Swapchain* pSwapchain = getSwapchain(swapchain);
		RenderProgram* pRenderProgram = getRenderProgram(renderProgram);

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

	bool Renderer::beginFrame(ResourceID swapchain) {
		Swapchain* pSwapchain = getSwapchain(swapchain);

		m_pCurrentCommandBufferSet = pSwapchain->beginFrame();
		if (!m_pCurrentCommandBufferSet) {
			return false;
		}

		return true;
	}

	void Renderer::endFrame(ResourceID swapchain) {
		Swapchain* pSwapchain = getSwapchain(swapchain);
		pSwapchain->endFrame(m_pCore->getGraphicsQueue());
	}

	void Renderer::beginRenderProgram(ResourceID renderProgram, ResourceID framebuffer, Color clearColor, Rect renderArea) {
		RenderProgram* pRenderProgram = getRenderProgram(renderProgram);
		FramebufferSet* pFramebuffer = getFramebufferSet(framebuffer);
		pRenderProgram->begin(m_pCurrentCommandBufferSet, pFramebuffer, clearColor, renderArea);
	}


	void Renderer::endRenderProgram(ResourceID renderProgram) {
		RenderProgram* pRenderProgram = getRenderProgram(renderProgram);
		pRenderProgram->end(m_pCurrentCommandBufferSet);
	}

}

