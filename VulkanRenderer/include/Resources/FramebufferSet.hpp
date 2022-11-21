#pragma once

#include "structs.hpp"

#include "Resources/Texture.hpp"

namespace sa {
	class VulkanCore;
	class Swapchain;

	class FramebufferSet {
	private:
		std::vector<vk::Framebuffer> m_buffers;
		std::vector<Texture> m_images;
		Extent m_extent;
		vk::Device m_device;

	public:
		FramebufferSet();
		FramebufferSet(VulkanCore* pCore, vk::RenderPass renderPass, const std::vector<Texture>& images, Extent extent, uint32_t layers);
		FramebufferSet(VulkanCore* pCore, vk::RenderPass renderPass, Swapchain* pSwapchain, const std::vector<Texture>& images, uint32_t layers);

		void create(VulkanCore* pCore, vk::RenderPass renderPass, const std::vector<Texture>& images, Extent extent, uint32_t layers);
		void create(VulkanCore* pCore, vk::RenderPass renderPass, Swapchain* pSwapchain, const std::vector<Texture>& images, uint32_t layers);
		
		void destroy();


		vk::Framebuffer getBuffer(uint32_t index) const;
		uint32_t getBufferCount() const;

		Extent getExtent() const;

		const Texture& getTexture(uint32_t index) const;
		size_t getTextureCount() const;

	};
}