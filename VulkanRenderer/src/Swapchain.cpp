#include "pch.h"
#include "Resources/Swapchain.hpp"

namespace sa {

	void Swapchain::createSyncronisationObjects() {

		m_imageFences.resize(m_images.size());
		
		uint32_t count = m_commandBufferSet.getBufferCount();
		m_inFlightFences.resize(count);
		for (size_t i = 0; i < count; i++) {
			m_inFlightFences[i] = m_device.createFence({ .flags = vk::FenceCreateFlagBits::eSignaled });
		}


		m_imageAvailableSemaphore.resize(count);
		for (size_t i = 0; i < count; i++) {
			m_imageAvailableSemaphore[i] = m_device.createSemaphore({});
		}

		m_renderFinishedSemaphore.resize(count);
		for (size_t i = 0; i < count; i++) {
			m_renderFinishedSemaphore[i] = m_device.createSemaphore({});
		}

	}

	Swapchain::Swapchain(VulkanCore* pCore, GLFWwindow* pWindow) {
		create(pCore, pWindow);
	}

	void Swapchain::create(VulkanCore* pCore, GLFWwindow* pWindow) {
		m_device = pCore->getDevice();
		m_instance = pCore->getInstance();
		m_physicalDevice = pCore->getPhysicalDevice();

		m_frameIndex = 0;

		m_surface = pCore->createSurface(pWindow);
		vk::Extent2D extent2D;
		m_swapchain = pCore->createSwapchain(m_surface, &m_format, &extent2D);

		m_extent = { extent2D.width, extent2D.height };

		m_images = pCore->getDevice().getSwapchainImagesKHR(m_swapchain);

		for (auto image : m_images) {
			m_imageViews.push_back(pCore->createImageView(
				vk::ImageViewType::e2D,
				image,
				m_format,
				vk::ImageAspectFlagBits::eColor,
				1,
				0,
				1,
				0
			));
		}
		
		m_commandBufferSet = pCore->allocateCommandBufferSet(vk::CommandBufferLevel::ePrimary);
		
		createSyncronisationObjects();

		SA_DEBUG_LOG_INFO("Created Swapchain\n\tImage count: ", m_images.size(), "\n\tFormat: ", vk::to_string(m_format));

	}

	void Swapchain::recreate(GLFWwindow* pWindow) {

	}

	void Swapchain::destroy() {

		m_commandBufferSet.destroy();

		for (auto fence : m_inFlightFences) {
			m_device.destroyFence(fence);
		}
		m_inFlightFences.clear();
		m_imageFences.clear();

		for (auto semaphore : m_imageAvailableSemaphore) {
			m_device.destroySemaphore(semaphore);
		}
		m_imageAvailableSemaphore.clear();

		for (auto semaphore : m_renderFinishedSemaphore) {
			m_device.destroySemaphore(semaphore);
		}
		m_renderFinishedSemaphore.clear();

		for (auto imageView : m_imageViews) {
			m_device.destroyImageView(imageView);
		}
		m_imageViews.clear();

		m_device.destroySwapchainKHR(m_swapchain);
		m_instance.destroySurfaceKHR(m_surface);
	}


	CommandBufferSet* Swapchain::beginFrame() {
		if (!m_swapchain) {
			return nullptr;
		}
		m_device.waitForFences(m_inFlightFences[m_frameIndex], VK_FALSE, UINT64_MAX);

		m_imageIndex = m_device.acquireNextImageKHR(m_swapchain, UINT64_MAX, m_imageAvailableSemaphore[m_frameIndex]).value;
		/*
		if (m_imageFences[m_imageIndex]) {
			checkError(
				m_device.waitForFences(m_imageFences[m_imageIndex], VK_FALSE, UINT64_MAX),
				"Failed to wait for image fence");
		}
		m_imageFences[m_imageIndex] = m_inFlightFences[m_frameIndex];
		*/
	
		m_commandBufferSet.begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

		return &m_commandBufferSet;
	}

	void Swapchain::endFrame() {

		m_commandBufferSet.end();
		

		m_device.resetFences(m_inFlightFences[m_frameIndex]);

		// Submit
		m_commandBufferSet.submit(m_inFlightFences[m_frameIndex], m_renderFinishedSemaphore[m_frameIndex], m_imageAvailableSemaphore[m_frameIndex]);

		// Present
		m_commandBufferSet.present(m_renderFinishedSemaphore[m_frameIndex], m_swapchain, m_imageIndex);

		m_frameIndex = (m_frameIndex + 1) % m_commandBufferSet.getBufferCount();

	}

	void Swapchain::waitForFrame() {
		m_device.waitForFences(m_inFlightFences[m_frameIndex], VK_FALSE, UINT64_MAX);
	}

	std::vector<vk::ImageView> Swapchain::getImageViews() const {
		return m_imageViews;
	}

	Extent Swapchain::getExtent() const {
		return m_extent;
	}

	vk::Format Swapchain::getFormat() const {
		return m_format;
	}

	uint32_t Swapchain::getImageCount() const {
		return m_images.size();
	}

	const CommandBufferSet* Swapchain::getCommandBufferSet() const {
		return &m_commandBufferSet;
	}

	uint32_t Swapchain::getFrameIndex() const {
		return m_frameIndex;
	}

	uint32_t Swapchain::getImageIndex() const {
		return m_imageIndex;
	}

}