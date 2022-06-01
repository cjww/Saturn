#include "pch.h"
#include "Resources/Texture.hpp"
#include "Resources\Swapchain.hpp"

#include "VulkanCore.hpp"
#include "Renderer.hpp"
namespace sa {
	Texture::Texture(VulkanCore* pCore)
		: m_pCore(pCore)
		, m_pImage(nullptr)
		, m_pStagingBuffer(nullptr)
		, m_view(NULL_RESOURCE)
	{
	}
	
	Extent Texture::getExtent() const {
		return { m_pImage->extent.width, m_pImage->extent.height };
	}

	uint32_t Texture::getDepth() const {
		return 1;
	}

	vk::ImageView* Texture::getView() const {
		return ResourceManager::get().get<vk::ImageView>(m_view);
	}

	TextureTypeFlags Texture::getTypeFlags() const {
		return m_type;
	}

	Texture2D::Texture2D(VulkanCore* pCore, TextureTypeFlags type, Extent extent, uint32_t sampleCount)
		: Texture(pCore)
	{
		m_type = type;
		create(type, extent, sampleCount);
	}

	Texture2D::Texture2D(VulkanCore* pCore, TextureTypeFlags type, Extent extent, FormatPrecisionFlags precisions, FormatDimensionFlags dimensions, FormatTypeFlags types, uint32_t sampleCount)
		: Texture(pCore)
	{
		m_type = type;
		create(type, extent, precisions, dimensions, types, sampleCount);
	}

	Texture2D::Texture2D(VulkanCore* pCore, TextureTypeFlags type, Extent extent, Swapchain* pSwapchain, uint32_t sampleCount) 
		: Texture(pCore)
	{
		m_type = type;
		create(type, extent, pSwapchain, sampleCount);
	}

	Texture2D::Texture2D(VulkanCore* pCore, const Image& image)
		: Texture(pCore)
	{
		m_type = TextureTypeFlagBits::SAMPLED | TextureTypeFlagBits::TRANSFER_DST;
		create(m_type, image.getExtent(), 1);
		
		m_pStagingBuffer = m_pCore->createBuffer(
			vk::BufferUsageFlagBits::eTransferSrc,
			VMA_MEMORY_USAGE_AUTO,
			VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
			image.getWidth() * image.getHeight() * image.getChannelCount(),
			image.getPixels());
			// transfer data
		DataTransfer transfer {
			.type = DataTransfer::Type::BUFFER_TO_IMAGE,
			.srcBuffer = m_pStagingBuffer,
			.dstImage = m_pImage,
		};
		Renderer::get().queueTransfer(transfer);
	}
	
	void Texture2D::create(TextureTypeFlags type, Extent extent, uint32_t sampleCount) {

		vk::ImageUsageFlags usage = (vk::ImageUsageFlags)type;
		vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor;

		vk::Format format = m_pCore->getDefaultColorFormat();

		if (type & TextureTypeFlagBits::DEPTH_ATTACHMENT) {
			aspect = vk::ImageAspectFlagBits::eDepth;
			format = m_pCore->getDefaultDepthFormat();
		}		

		DEBUG_LOG_INFO("Created 2D texture\nFormat: ", vk::to_string(format));
		m_pImage = m_pCore->createImage2D(
			extent,
			format,
			usage,
			(vk::SampleCountFlagBits)sampleCount,
			1,
			1
		);


		m_view = ResourceManager::get().insert<vk::ImageView>(m_pCore->createImageView(
			vk::ImageViewType::e2D,
			m_pImage->image,
			format,
			aspect,
			0,
			0
		));

	}

	void Texture2D::create(TextureTypeFlags type, Extent extent, FormatPrecisionFlags precisions, FormatDimensionFlags dimensions, FormatTypeFlags types, uint32_t sampleCount) {
		vk::ImageUsageFlags usage = (vk::ImageUsageFlags)type;
		vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor;

		vk::Format format = vk::Format::eUndefined;
		vk::FormatFeatureFlags features;
		if (type & TextureTypeFlagBits::DEPTH_ATTACHMENT) {
			features |= vk::FormatFeatureFlagBits::eDepthStencilAttachment;
			aspect = vk::ImageAspectFlagBits::eDepth;
		}
		if(type & TextureTypeFlagBits::SAMPLED) {
			features |= vk::FormatFeatureFlagBits::eSampledImage;
		}
		if (type & TextureTypeFlagBits::COLOR_ATTACHMENT) {
			features |= vk::FormatFeatureFlagBits::eColorAttachment;
		}
		if (type & TextureTypeFlagBits::STORAGE) {
			features |= vk::FormatFeatureFlagBits::eStorageImage;
		}
		if (type & TextureTypeFlagBits::TRANSFER_DST) {
			features |= vk::FormatFeatureFlagBits::eTransferDst;
		}

		format = m_pCore->getFormat(precisions, dimensions, types, 
			features, vk::ImageTiling::eOptimal);
		if (format == vk::Format::eUndefined) {
			DEBUG_LOG_WARNING("No supported format found, using default format");
			if (aspect == vk::ImageAspectFlagBits::eColor) {
				format = m_pCore->getDefaultColorFormat();
			}
			else {
				format = m_pCore->getDefaultDepthFormat();
			}
		}

		DEBUG_LOG_INFO("Created 2D texture\nFormat: ", vk::to_string(format));
		m_pImage = m_pCore->createImage2D(
			extent,
			format,
			usage,
			(vk::SampleCountFlagBits)sampleCount,
			1,
			1
		);

		m_view = ResourceManager::get().insert<vk::ImageView>(m_pCore->createImageView(
			vk::ImageViewType::e2D,
			m_pImage->image,
			format,
			aspect,
			0,
			0
		));

	}

	void Texture2D::create(TextureTypeFlags type, Extent extent, Swapchain* pSwapchain, uint32_t sampleCount) {

		vk::ImageUsageFlags usage = (vk::ImageUsageFlags)type;
		
		DEBUG_LOG_INFO("Created 2D texture\nFormat: ", vk::to_string(pSwapchain->getFormat()));

		m_pImage = m_pCore->createImage2D(
			extent,
			pSwapchain->getFormat(),
			usage,
			(vk::SampleCountFlagBits)sampleCount,
			1,
			1
		);


		m_view = ResourceManager::get().insert<vk::ImageView>(m_pCore->createImageView(
			vk::ImageViewType::e2D,
			m_pImage->image,
			pSwapchain->getFormat(),
			vk::ImageAspectFlagBits::eColor,
			0,
			0
		));

	}

	void Texture2D::destroy() {
		m_pCore->getDevice().waitIdle();
		if (m_pStagingBuffer)
			m_pCore->destroyBuffer(m_pStagingBuffer);

		ResourceManager::get().remove<vk::ImageView>(m_view);
		m_pCore->destroyImage(m_pImage);
		m_view = NULL_RESOURCE;
	}


}