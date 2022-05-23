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
			m_pCore = std::make_unique<VulkanCore>();
			m_pCore->init();

			ResourceManager::get().setCleanupFunction<Swapchain>([](Swapchain* s) { s->destroy(); });

		}
		catch (const std::exception& e) {
			DEBUG_LOG_ERROR(e.what());
		}
	}
	Renderer::~Renderer() {
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

	ResourceID Renderer::createFramebuffer(ResourceID renderProgram, const std::vector<Texture>& attachmentTextures) {
		RenderProgram* pRenderProgram = ResourceManager::get().get<RenderProgram>(renderProgram);
		if (!pRenderProgram)
			throw std::runtime_error("Nonexistent swapchain: " + renderProgram);

		Extent extent = pSwapchain->getExtent();

		return ResourceManager::get().insert<FramebufferSet>(m_pCore->createFrameBufferSet(pRenderProgram->getRenderPass(), framebufferViews, extent.width, extent.height, layers));


	}

	ResourceID Renderer::createSwapchainFramebuffer(ResourceID swapchain, ResourceID renderProgram, const std::vector<Texture>& additionalAttachmentTextures, uint32_t layers) {
		Swapchain* pSwapchain = ResourceManager::get().get<Swapchain>(swapchain);
		if (!pSwapchain)
			throw std::runtime_error("Nonexistent swapchain: " + swapchain);
		RenderProgram* pRenderProgram = ResourceManager::get().get<RenderProgram>(renderProgram);
		if (!pRenderProgram)
			throw std::runtime_error("Nonexistent swapchain: " + renderProgram);

		std::vector<vk::ImageView> swapchainViews = pSwapchain->getImageViews();
		uint32_t count = swapchainViews.size();
		std::vector<std::vector<vk::ImageView>> framebufferViews(count);
		for (uint32_t i = 0; i < count; i++) {
			framebufferViews[i].resize(additionalAttachmentTextures.size() + 1);
			framebufferViews[i][0] = swapchainViews[i];
			for (uint32_t j = 1; j < (uint32_t)additionalAttachmentTextures.size(); j++) {
				framebufferViews[i][j] = *additionalAttachmentTextures[j].getView();
			}
		}

		Extent extent = pSwapchain->getExtent();
		return ResourceManager::get().insert<FramebufferSet>(m_pCore->createFrameBufferSet(pRenderProgram->getRenderPass(), framebufferViews, extent.width, extent.height, layers));
	}

	bool Renderer::beginFrame(ResourceID swapchain) {
		Swapchain* pSwapchain = ResourceManager::get().get<Swapchain>(swapchain);
		if (!pSwapchain)
			throw std::runtime_error("Nonexistent swapchain: " + swapchain);

		if (pSwapchain->beginFrame() == -1) {
			return false;
		}



		return true;
	}

	void Renderer::endFrame(ResourceID swapchain) {
		Swapchain* pSwapchain = ResourceManager::get().get<Swapchain>(swapchain);
		if (!pSwapchain)
			throw std::runtime_error("Nonexistent swapchain: " + swapchain);

		pSwapchain->endFrame(m_pCore->getGraphicsQueue());
	}

}

