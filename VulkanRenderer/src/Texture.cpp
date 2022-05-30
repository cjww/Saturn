#include "pch.h"
#include "Resources/Texture.hpp"

#include "VulkanCore.hpp"
#include "Renderer.hpp"
namespace sa {
	Texture::Texture(VulkanCore* pCore)
		: m_pCore(pCore)
		, m_pImage(nullptr)
		, m_pStagingBuffer(nullptr)
		, m_pView(nullptr)
	{
	}
	
	Extent Texture::getExtent() const {
		return { m_pImage->extent.width, m_pImage->extent.height };
	}

	uint32_t Texture::getDepth() const {
		return 1;
	}

	vk::ImageView* Texture::getView() const {
		return m_pView.get();
	}

	TextureTypeFlags Texture::getTypeFlags() const {
		return m_type;
	}

	Texture2D::Texture2D(VulkanCore* pCore, TextureTypeFlags type, Extent extent)
		: Texture(pCore)
	{
		m_type = type;
		create(type, extent);
	}

	Texture2D::Texture2D(VulkanCore* pCore, TextureTypeFlags type, Extent extent, FormatPrecisionFlags precisions, FormatDimensionFlags dimensions, FormatTypeFlags types)
		: Texture(pCore)
	{
		m_type = type;
		create(type, extent, precisions, dimensions, types);
	}

	Texture2D::Texture2D(VulkanCore* pCore, const Image& image)
		: Texture(pCore)
	{
		m_type = TextureTypeFlagBits::SAMPLED | TextureTypeFlagBits::TRANSFER_DST;
		create(m_type, image.getExtent());
		
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
	
	void Texture2D::create(TextureTypeFlags type, Extent extent) {

		vk::ImageUsageFlags usage = (vk::ImageUsageFlags)type;
		vk::Format format = m_pCore->getDefaultColorFormat();
		vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor;

		vk::ImageLayout layout = vk::ImageLayout::eUndefined;

		if (type & TextureTypeFlagBits::DEPTH_ATTACHMENT) {
			format = m_pCore->getDefaultDepthFormat();
			aspect = vk::ImageAspectFlagBits::eDepth;
		}
		if (type & TextureTypeFlagBits::STORAGE) {
			layout = vk::ImageLayout::eGeneral;
		}

		DEBUG_LOG_INFO("Created 2D texture\nFormat: ", vk::to_string(format));
		m_pImage = m_pCore->createImage2D(
			extent,
			format,
			usage,
			vk::SampleCountFlagBits::e1,
			1,
			1
		);

		m_pView = std::make_shared<vk::ImageView>(m_pCore->createImageView(
			vk::ImageViewType::e2D,
			m_pImage->image,
			format,
			aspect,
			0,
			0
		));
	}

	void Texture2D::create(TextureTypeFlags type, Extent extent, FormatPrecisionFlags precisions, FormatDimensionFlags dimensions, FormatTypeFlags types) {
		vk::ImageUsageFlags usage = (vk::ImageUsageFlags)type;
		vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor;

		vk::ImageLayout layout = vk::ImageLayout::eUndefined;

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
			layout = vk::ImageLayout::eGeneral;
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
			vk::SampleCountFlagBits::e1,
			1,
			1
		);

		m_pView = std::make_shared<vk::ImageView>(m_pCore->createImageView(
			vk::ImageViewType::e2D,
			m_pImage->image,
			format,
			aspect,
			0,
			0
		));

	}

	void Texture2D::destroy() {
		m_pCore->getDevice().waitIdle();
		if (m_pStagingBuffer)
			m_pCore->destroyBuffer(m_pStagingBuffer);
		m_pCore->getDevice().destroyImageView(*m_pView);
		m_pCore->destroyImage(m_pImage);
		m_pView = nullptr;
	}


}