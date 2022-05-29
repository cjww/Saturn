#pragma once

#include "VulkanCore.hpp"
#include "debugFunctions.h"

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
		CommandBufferSet m_computeCommandBufferSet; // since we dont know if the main commandBUffer set can submit compute commands

		
		std::vector<vk::Semaphore> m_imageAvailableSemaphore;
		std::vector<vk::Semaphore> m_renderFinishedSemaphore;
		std::vector<vk::Fence> m_inFlightFences;
		std::vector<vk::Fence> m_imageFences;

		std::vector<vk::Fence> m_inFlightFencesCompute;

		uint32_t m_frameIndex;
		uint32_t m_imageIndex;

		std::function<void(Extent newExtent)> m_resizeCallback;

		vk::Device m_device;
		vk::PhysicalDevice m_physicalDevice;
		vk::Instance m_instance;

		vk::Queue m_graphicsQueue;

		Extent m_extent;
		vk::Format m_format;

		void createSyncronisationObjects();


	public:
		Swapchain() = default;

		void create(VulkanCore* pCore, GLFWwindow* pWindow);

		void destroy();

		void setResizeCallback(std::function<void(Extent)> function);


		std::tuple<CommandBufferSet*, CommandBufferSet*> beginFrame();
		void endFrame();

		std::vector<vk::ImageView> getImageViews() const;
		Extent getExtent() const;
		vk::Format getFormat() const;

		uint32_t getImageCount() const;

	};
}