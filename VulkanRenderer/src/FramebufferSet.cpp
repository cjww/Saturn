#include "pch.h"
#include "internal/FramebufferSet.hpp"

#include "internal/VulkanCore.hpp"
#include "internal/Swapchain.hpp"

namespace sa{
	FramebufferSet::FramebufferSet() 
		: m_extent({0, 0})
		, m_pSwapchain(nullptr)
	{
	}

	FramebufferSet::FramebufferSet(VulkanCore* pCore, vk::RenderPass renderPass, const std::vector<DynamicTexture>& images, Extent extent, uint32_t layers) 
		: FramebufferSet()
	{
		m_isDynamic = true;
		m_device = pCore->getDevice();
		m_extent = extent;
		
		m_dynamicImages = images;

		create(pCore, renderPass, images, extent, layers);
	}


	FramebufferSet::FramebufferSet(VulkanCore* pCore, vk::RenderPass renderPass, Swapchain* pSwapchain, const std::vector<DynamicTexture>& images, uint32_t layers)
		: FramebufferSet()
	{
		m_isDynamic = true;
		m_device = pCore->getDevice();
		m_extent = pSwapchain->getExtent();

		m_dynamicImages = images;

		m_pSwapchain = pSwapchain;
		create(pCore, renderPass, pSwapchain, images, layers);
	}

	FramebufferSet::FramebufferSet(VulkanCore* pCore, vk::RenderPass renderPass, const std::vector<Texture>& images, Extent extent, uint32_t layers)
		: FramebufferSet()
	{
		m_isDynamic = false;
		m_device = pCore->getDevice();
		m_extent = extent;

		m_images = images;

		create(pCore, renderPass, images, extent, layers);
	}


	FramebufferSet::FramebufferSet(VulkanCore* pCore, vk::RenderPass renderPass, Swapchain* pSwapchain, const std::vector<Texture>& images, uint32_t layers)
		: FramebufferSet()
	{
		m_isDynamic = false;
		m_device = pCore->getDevice();
		m_extent = pSwapchain->getExtent();

		m_images = images;

		m_pSwapchain = pSwapchain;
		create(pCore, renderPass, pSwapchain, images, layers);
	}



	void FramebufferSet::create(VulkanCore* pCore, vk::RenderPass renderPass, const std::vector<DynamicTexture>& images, Extent extent, uint32_t layers) {
		if (m_buffers.size() > 0)
			destroy();
		
		std::vector<std::vector<vk::ImageView>> framebufferViews(pCore->getQueueCount());
		for (uint32_t i = 0; i < (uint32_t)framebufferViews.size(); i++) {
			framebufferViews[i].resize(images.size());
			for (uint32_t j = 0; j < images.size(); j++) {
				auto& texture = images[j].getTexture(i);
				if (extent.width != texture.getExtent().width || extent.height != texture.getExtent().height) {
					throw std::runtime_error("All attachments must be of the same size");
				}
				framebufferViews[i][j] = *texture.getView();
				
			}
		}

		m_buffers.resize(framebufferViews.size());
		for (uint32_t i = 0; i < (uint32_t)framebufferViews.size(); i++) {
			m_buffers[i] = pCore->createFrameBuffer(renderPass, framebufferViews[i], extent.width, extent.height, layers);
		}
	}

	void FramebufferSet::create(VulkanCore* pCore, vk::RenderPass renderPass, Swapchain* pSwapchain, const std::vector<DynamicTexture>& images, uint32_t layers) {
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
				auto& texture = images[j - 1].getTexture(i);
				if (texture.isValidImage() && (extent.width != texture.getExtent().width || extent.height != texture.getExtent().height)) {
					throw std::runtime_error("All attachments must be of the same size");
				}
				framebufferViews[i][j] = *texture.getView();
			}
		}

		m_buffers.resize(framebufferViews.size());
		for (uint32_t i = 0; i < framebufferViews.size(); i++) {
			m_buffers[i] = pCore->createFrameBuffer(renderPass, framebufferViews[i], extent.width, extent.height, layers);
		}
	}


	void FramebufferSet::create(VulkanCore* pCore, vk::RenderPass renderPass, const std::vector<Texture>& images, Extent extent, uint32_t layers) {
		if (m_buffers.size() > 0)
			destroy();

		std::vector<std::vector<vk::ImageView>> framebufferViews(pCore->getQueueCount());
		for (uint32_t i = 0; i < (uint32_t)framebufferViews.size(); i++) {
			for (auto& texture : images) {
				if (texture.isValidImage() && (extent.width != texture.getExtent().width || extent.height != texture.getExtent().height)) {
					throw std::runtime_error("All attachments must be of the same size");
				}
				framebufferViews[i].push_back(*texture.getView());
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
				Texture texture = images[j - 1];
				if (extent.width != texture.getExtent().width || extent.height != texture.getExtent().height) {
					throw std::runtime_error("All attachments must be of the same size");
				}
				framebufferViews[i][j] = *texture.getView();
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

	void FramebufferSet::swap() {
		for (auto& image : m_dynamicImages) {
			image.swap();
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

	const Texture& FramebufferSet::getTexture(uint32_t attachmentIndex) const {
		return m_isDynamic ?
			m_dynamicImages.at(attachmentIndex).getTexture() :
			m_images.at(attachmentIndex);
	}

	const DynamicTexture& FramebufferSet::getDynamicTexture(uint32_t attachmentIndex) const {
		return m_dynamicImages.at(attachmentIndex);
	}

	DynamicTexture* FramebufferSet::getDynamicTexturePtr(uint32_t attachmentIndex) {
		return &m_dynamicImages.at(attachmentIndex);
	}

	size_t FramebufferSet::getTextureCount() const {
		return m_isDynamic ?
			m_dynamicImages.size() :
			m_images.size();
	}

	Swapchain* FramebufferSet::getSwapchain() const {
		return m_pSwapchain;
	}

}