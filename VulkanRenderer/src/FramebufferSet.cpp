#include "pch.h"
#include "Resources/FramebufferSet.hpp"

#include "VulkanCore.hpp"
#include "Resources\Swapchain.hpp"

namespace sa{
	FramebufferSet::FramebufferSet() 
		: m_extent({0, 0})
	{
	}

	FramebufferSet::FramebufferSet(VulkanCore* pCore, vk::RenderPass renderPass, const std::vector<Texture>& images, Extent extent, uint32_t layers) {
		m_device = pCore->getDevice();
		m_extent = extent;
		
		for (const Texture& image : images) {
			m_images.push_back((DeviceImage*)image);
		}

		create(pCore, renderPass, images, extent, layers);
	}


	FramebufferSet::FramebufferSet(VulkanCore* pCore, vk::RenderPass renderPass, Swapchain* pSwapchain, const std::vector<Texture>& images, uint32_t layers) {
		m_device = pCore->getDevice();
		m_extent = pSwapchain->getExtent();

		for (const Texture& image : images) {
			m_images.push_back((DeviceImage*)image);
		}

		create(pCore, renderPass, pSwapchain, images, layers);
	}


	void FramebufferSet::create(VulkanCore* pCore, vk::RenderPass renderPass, const std::vector<Texture>& images, Extent extent, uint32_t layers) {
		if (m_buffers.size() > 0)
			destroy();
		
		std::vector<std::vector<vk::ImageView>> framebufferViews(pCore->getQueueCount());
		for (uint32_t i = 0; i < (uint32_t)framebufferViews.size(); i++) {
			for (auto& texture : images) {
				framebufferViews[i].push_back(*texture.getView());
				if (extent.width != texture.getExtent().width || extent.height != texture.getExtent().height) {
					throw std::runtime_error("All attachments must be of the same size");
				}
			}
		}

		m_buffers.resize(framebufferViews.size());
		for (uint32_t i = 0; i < (uint32_t)framebufferViews.size(); i++) {
			m_buffers[i] = pCore->createFrameBuffer(renderPass, framebufferViews[i], extent.width, extent.height, layers);
		}
	}

	void FramebufferSet::create(VulkanCore* pCore, vk::RenderPass renderPass, Swapchain* pSwapchain, const std::vector<Texture>& images, uint32_t layers) {
		if (m_buffers.size() > 0)
			destroy();

		Extent extent = pSwapchain->getExtent();

		std::vector<vk::ImageView> swapchainViews = pSwapchain->getImageViews();
		uint32_t count = static_cast<uint32_t>(swapchainViews.size());
		std::vector<std::vector<vk::ImageView>> framebufferViews(count);
		for (uint32_t i = 0; i < count; i++) {
			framebufferViews[i].resize(images.size() + 1);
			framebufferViews[i][0] = swapchainViews[i];
			for (uint32_t j = 1; j < (uint32_t)framebufferViews[i].size(); j++) {
				framebufferViews[i][j] = *images[j - 1].getView();
				if (extent.width != images[j - 1].getExtent().width || extent.height != images[j - 1].getExtent().height) {
					throw std::runtime_error("All attachments must be of the same size");
				}
			}
		}

		m_buffers.resize(framebufferViews.size());
		for (uint32_t i = 0; i < framebufferViews.size(); i++) {
			m_buffers[i] = pCore->createFrameBuffer(renderPass, framebufferViews[i], extent.width, extent.height, layers);
		}
	}

	void FramebufferSet::destroy() {
		for (auto& framebuffer : m_buffers) {
			m_device.destroyFramebuffer(framebuffer);
		}
	}

	vk::Framebuffer FramebufferSet::getBuffer(uint32_t index) const {
		return m_buffers.at(index);
	}

	uint32_t FramebufferSet::getBufferCount() const {
		return (uint32_t)m_buffers.size();
	}


	Extent FramebufferSet::getExtent() const {
		return m_extent;
	}

}