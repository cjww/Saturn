#pragma once

#include "VulkanCore.hpp"
#include "debugFunctions.hpp"

#include "CommandPool.hpp"

#include "RenderProgram.hpp"
#include "structs.hpp"

namespace sa {

	class Swapchain {
	private:
		vk::SurfaceKHR m_surface;
		vk::SwapchainKHR m_swapchain;
		std::vector<vk::Image> m_images;
		std::vector<vk::ImageView> m_imageViews;

		CommandBufferSet m_commandBufferSet;
		
		std::vector<vk::Semaphore> m_imageAvailableSemaphore;
		std::vector<vk::Semaphore> m_renderFinishedSemaphore;
		std::vector<vk::Fence> m_inFlightFences;
		std::vector<vk::Fence> m_imageFences;

		uint32_t m_frameIndex;
		uint32_t m_imageIndex;

		vk::Device m_device;
		vk::PhysicalDevice m_physicalDevice;
		vk::Instance m_instance;

		Extent m_extent;
		vk::Format m_format;

		void createSyncronisationObjects();


	public:
		Swapchain() = default;
		Swapchain(VulkanCore* pCore, GLFWwindow* pWindow);

		void create(VulkanCore* pCore, GLFWwindow* pWindow);
		void recreate(GLFWwindow* pWindow);
		void destroy();


		CommandBufferSet* beginFrame();
		void endFrame();

		void waitForFrame();

		std::vector<vk::ImageView> getImageViews() const;
		vk::Image getImage(uint32_t imageIndex) const;

		Extent getExtent() const;
		vk::Format getFormat() const;

		uint32_t getImageCount() const;

		const CommandBufferSet* getCommandBufferSet() const;
	
		uint32_t getFrameIndex() const;
		uint32_t getImageIndex() const;

	};
}