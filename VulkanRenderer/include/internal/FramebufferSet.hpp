#pragma once

#include "structs.hpp"

#include "Resources/Texture.hpp"
#include "Resources/DynamicTexture.hpp"

namespace sa {
	class VulkanCore;
	class Swapchain;

	class FramebufferSet {
	private:
		std::vector<vk::Framebuffer> m_buffers;
		std::vector<Texture> m_images;
		std::vector<DynamicTexture> m_dynamicImages;
		bool m_isDynamic;

		Extent m_extent;
		vk::Device m_device;

		Swapchain* m_pSwapchain;

	public:
		FramebufferSet();
		FramebufferSet(VulkanCore* pCore, vk::RenderPass renderPass, const DynamicTexture* pImages, uint32_t imageCount, Extent extent, uint32_t layers);
		FramebufferSet(VulkanCore* pCore, vk::RenderPass renderPass, Swapchain* pSwapchain, const DynamicTexture* pImages, uint32_t imageCount, uint32_t layers);

		FramebufferSet(VulkanCore* pCore, vk::RenderPass renderPass, const Texture* pImages, uint32_t imageCount, Extent extent, uint32_t layers);
		FramebufferSet(VulkanCore* pCore, vk::RenderPass renderPass, Swapchain* pSwapchain, const Texture* pImages, uint32_t imageCount, uint32_t layers);


		void create(VulkanCore* pCore, vk::RenderPass renderPass, const DynamicTexture* pImages, uint32_t imageCount, Extent extent, uint32_t layers);
		void create(VulkanCore* pCore, vk::RenderPass renderPass, Swapchain* pSwapchain, const DynamicTexture* pImages, uint32_t imageCount, uint32_t layers);
		
		void create(VulkanCore* pCore, vk::RenderPass renderPass, const Texture* pImages, uint32_t imageCount, Extent extent, uint32_t layers);
		void create(VulkanCore* pCore, vk::RenderPass renderPass, Swapchain* pSwapchain, const Texture* pImages, uint32_t imageCount, uint32_t layers);

		void destroy();

		void sync(const RenderContext& context);

		vk::Framebuffer getBuffer(uint32_t index) const;
		uint32_t getBufferCount() const;

		Extent getExtent() const;

		const Texture& getTexture(uint32_t attachmentIndex) const;
		const DynamicTexture& getDynamicTexture(uint32_t attachmentIndex) const;
		DynamicTexture* getDynamicTexturePtr(uint32_t attachmentIndex);

		size_t getTextureCount() const;

		Swapchain* getSwapchain() const;
	};
}