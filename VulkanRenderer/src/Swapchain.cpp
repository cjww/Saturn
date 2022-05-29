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

		if (m_computeCommandBufferSet.isValid()) {
			m_inFlightFencesCompute.resize(count);
			for (size_t i = 0; i < count; i++) {
				m_inFlightFencesCompute[i] = m_device.createFence({ .flags = vk::FenceCreateFlagBits::eSignaled });
			}
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


		m_commandBufferSet = pCore->allocateGraphicsCommandBufferSet(static_cast<uint32_t>(m_images.size()), vk::CommandBufferLevel::ePrimary);
		if (m_commandBufferSet.getTargetQueue() != pCore->getComputeQueue()) {
			m_computeCommandBufferSet = pCore->allocateComputeCommandBufferSet(static_cast<uint32_t>(m_images.size()), vk::CommandBufferLevel::ePrimary);
		}

		createSyncronisationObjects();

		DEBUG_LOG_INFO("Created Swapchain\n\tImage count: ", m_images.size(), "\n\tFormat: ", vk::to_string(m_format));

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

		for (auto fence : m_inFlightFencesCompute) {
			m_device.destroyFence(fence);
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

	std::tuple<CommandBufferSet*, CommandBufferSet*> Swapchain::beginFrame() {
		if (!m_swapchain) {
			return { nullptr, nullptr };
		}
		checkError(
			m_device.waitForFences(m_inFlightFences[m_frameIndex], VK_FALSE, UINT64_MAX),
			"Failed to wait for in flight fence");

		if (m_computeCommandBufferSet.isValid()) {
			checkError(
				m_device.waitForFences(m_inFlightFencesCompute[m_frameIndex], VK_FALSE, UINT64_MAX),
				"Failed to wait for in flight fence");
		}

		vk::ResultValue<uint32_t> res = m_device.acquireNextImageKHR(m_swapchain, UINT64_MAX, m_imageAvailableSemaphore[m_frameIndex]);
		if (res.result == vk::Result::eErrorOutOfDateKHR || res.result == vk::Result::eSuboptimalKHR) {
			if (m_resizeCallback) {
				vk::SurfaceCapabilitiesKHR capabilities = m_physicalDevice.getSurfaceCapabilitiesKHR(m_surface);
				Extent newExtent = { capabilities.currentExtent.width, capabilities.currentExtent.height };
				m_resizeCallback(newExtent);
				m_extent = newExtent;
			}
			return { nullptr, nullptr };
		}
		checkError(
			res.result,
			"Failed to acquire image",
			true
		);
		m_imageIndex = res.value;
		if (m_imageFences[m_imageIndex]) {
			checkError(
				m_device.waitForFences(m_imageFences[m_imageIndex], VK_FALSE, UINT64_MAX),
				"Failed to wait for image fence");
		}
		m_imageFences[m_imageIndex] = m_inFlightFences[m_frameIndex];
	
		m_commandBufferSet.begin(m_frameIndex, vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		if (m_computeCommandBufferSet.isValid()) {
			m_computeCommandBufferSet.begin(m_frameIndex, vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
			
			return { &m_commandBufferSet, &m_computeCommandBufferSet };
		}

		return { &m_commandBufferSet, nullptr};
	}

	void Swapchain::endFrame() {

		m_commandBufferSet.end();
		

		m_device.resetFences(m_inFlightFences[m_frameIndex]);


		// Submit
		if (m_computeCommandBufferSet.isRecording()) {
			m_computeCommandBufferSet.end();
			m_device.resetFences(m_inFlightFencesCompute[m_frameIndex]);
			m_computeCommandBufferSet.submit(m_inFlightFencesCompute[m_frameIndex]);
		}
		m_commandBufferSet.submit(m_inFlightFences[m_frameIndex], m_renderFinishedSemaphore[m_frameIndex], m_imageAvailableSemaphore[m_frameIndex]);

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
		checkError(m_commandBufferSet.getTargetQueue().presentKHR(presentInfo), "Failed to present", false);
		checkError(result, "Failed to present", false);

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

	uint32_t Swapchain::getImageCount() const {
		return m_images.size();
	}

}