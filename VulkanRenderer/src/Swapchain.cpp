#include "pch.h"
#include "Resources/Swapchain.hpp"

namespace sa {

	void Swapchain::createSyncronisationObjects() {

		size_t count = m_images.size();

		m_inFlightFences.resize(count);
		for (size_t i = 0; i < count; i++) {
			m_inFlightFences[i] = m_device.createFence({ .flags = vk::FenceCreateFlagBits::eSignaled });
		}

		m_imageFences.resize(count);

		m_imageAvailableSemaphore.resize(count);
		for (size_t i = 0; i < count; i++) {
			m_imageAvailableSemaphore[i] = m_device.createSemaphore({});
		}

		m_renderFinishedSemaphore.resize(count);
		for (size_t i = 0; i < count; i++) {
			m_renderFinishedSemaphore[i] = m_device.createSemaphore({});
		}
	}

	void Swapchain::create(VulkanCore* pCore, GLFWwindow* pWindow) {
		m_device = pCore->getDevice();
		m_instance = pCore->getInstance();
		m_physicalDevice = pCore->getPhysicalDevice();

		m_frameIndex = 0;

		m_surface = pCore->createSurface(pWindow);
		m_swapchain = pCore->createSwapchain(m_surface, pCore->getGraphicsQueueFamily(), &m_format);

		vk::SurfaceCapabilitiesKHR surfaceCapabilties = pCore->getPhysicalDevice().getSurfaceCapabilitiesKHR(m_surface);
		m_extent = { surfaceCapabilties.currentExtent.width, surfaceCapabilties.currentExtent.height };

		m_images = pCore->getDevice().getSwapchainImagesKHR(m_swapchain);

		for (auto image : m_images) {
			m_imageViews.push_back(pCore->createImageView(
				vk::ImageViewType::e2D,
				image,
				m_format,
				vk::ImageAspectFlagBits::eColor,
				0,
				0
			));
		}

		createSyncronisationObjects();

		m_commandBufferSet = pCore->allocateGraphicsCommandBufferSet(m_images.size(), vk::CommandBufferLevel::ePrimary);

	}

	void Swapchain::destroy() {

		for (auto fence : m_inFlightFences) {
			m_device.destroyFence(fence);
		}

		for (auto semaphore : m_imageAvailableSemaphore) {
			m_device.destroySemaphore(semaphore);
		}
		for (auto semaphore : m_renderFinishedSemaphore) {
			m_device.destroySemaphore(semaphore);
		}

		for (auto imageView : m_imageViews) {
			m_device.destroyImageView(imageView);
		}

		m_device.destroySwapchainKHR(m_swapchain);
		m_instance.destroySurfaceKHR(m_surface);

	}

	void Swapchain::setResizeCallback(std::function<void(Extent)> function) {
		m_resizeCallback = function;
	}

	CommandBufferSet* Swapchain::beginFrame() {
		if (m_swapchain == VK_NULL_HANDLE) {
			return nullptr;
		}
		m_device.waitForFences(m_inFlightFences[m_frameIndex], VK_FALSE, UINT64_MAX);

		vk::ResultValue<uint32_t> res = m_device.acquireNextImageKHR(m_swapchain, UINT64_MAX, m_imageAvailableSemaphore[m_frameIndex]);
		if (res.result == vk::Result::eErrorOutOfDateKHR || res.result == vk::Result::eSuboptimalKHR) {
			if (m_resizeCallback) {
				vk::SurfaceCapabilitiesKHR capabilities = m_physicalDevice.getSurfaceCapabilitiesKHR(m_surface);
				Extent newExtent = { capabilities.currentExtent.width, capabilities.currentExtent.height };
				m_resizeCallback(newExtent);
				m_extent = newExtent;
			}
			return nullptr;
		}
		else {
			checkError(
				res.result,
				"Failed to acquire image",
				true
			);
			m_imageIndex = res.value;
			if (m_imageFences[m_imageIndex] != VK_NULL_HANDLE) {
				m_device.waitForFences(m_imageFences[m_imageIndex], VK_FALSE, UINT64_MAX);
			}
			m_imageFences[m_imageIndex] = m_inFlightFences[m_frameIndex];
		}

		m_commandBufferSet.begin(m_frameIndex);

		return &m_commandBufferSet;
	}

	void Swapchain::endFrame(vk::Queue queue) {

		m_commandBufferSet.end();

		m_device.resetFences(m_inFlightFences[m_frameIndex]);

		// Submit
		m_commandBufferSet.submit(queue, m_inFlightFences[m_frameIndex], m_renderFinishedSemaphore[m_frameIndex], m_imageAvailableSemaphore[m_frameIndex]);

		// Present
		vk::Result result;
		vk::PresentInfoKHR presentInfo{
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &m_renderFinishedSemaphore[m_frameIndex],
			.swapchainCount = 1,
			.pSwapchains = &m_swapchain,
			.pImageIndices = &m_imageIndex,
			.pResults = &result
		};
		checkError(result, "Failed to present", false);
		checkError(queue.presentKHR(presentInfo), "Failed to present", false);

		m_frameIndex = (m_frameIndex + 1) % static_cast<uint32_t>(m_images.size());

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

}