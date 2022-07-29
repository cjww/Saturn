#include "pch.h"
#include "Resources/Texture.hpp"
#include "Resources\Swapchain.hpp"

#include "VulkanCore.hpp"
#include "Renderer.hpp"

#include "Resources/DeviceMemoryManager.hpp"

namespace sa {
	Texture::Texture(VulkanCore* pCore)
		: m_pCore(pCore)
		, m_pImage(nullptr)
		, m_pStagingBuffer(nullptr)
		, m_view(NULL_RESOURCE)
	{
	}
	
	Texture::Texture() : Texture(nullptr) {

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

	bool Texture::isValid() const {
		return m_pImage != nullptr;
	}

	void Texture::destroy() {
		if (!isValid())
			return;
		m_pCore->getDevice().waitIdle();
		if (m_pStagingBuffer)
			m_pCore->destroyBuffer(m_pStagingBuffer);

		ResourceManager::get().remove<vk::ImageView>(m_view);
		m_pCore->destroyImage(m_pImage);
		m_view = NULL_RESOURCE;
	}

	Texture2D::Texture2D(VulkanCore* pCore, TextureTypeFlags type, Extent extent, uint32_t sampleCount)
		: Texture(pCore)
	{
		m_type = type;
		create(type, extent, sampleCount, 1);
	}

	Texture2D::Texture2D(VulkanCore* pCore, TextureTypeFlags type, Extent extent, FormatPrecisionFlags precisions, FormatDimensionFlags dimensions, FormatTypeFlags types, uint32_t sampleCount)
		: Texture(pCore)
	{
		m_type = type;
		create(type, extent, precisions, dimensions, types, sampleCount, 1);
	}

	Texture2D::Texture2D(VulkanCore* pCore, TextureTypeFlags type, Extent extent, Swapchain* pSwapchain, uint32_t sampleCount) 
		: Texture(pCore)
	{
		m_type = type;
		create(type, extent, pSwapchain, sampleCount);
	}

	Texture2D::Texture2D(VulkanCore* pCore, const Image& image, bool generateMipmaps)
		: Texture(pCore)
	{
		m_type = TextureTypeFlagBits::SAMPLED | TextureTypeFlagBits::TRANSFER_DST;

		uint32_t mipLevels = 1;
		if (generateMipmaps) {
			mipLevels = image.calculateMipLevelCount();
			m_type |= TextureTypeFlagBits::TRANSFER_SRC;
		}

		create(
			m_type, 
			image.getExtent(), 
			sa::FormatPrecisionFlagBits::e8Bit, 
			sa::FormatDimensionFlagBits::e4, 
			sa::FormatTypeFlagBits::ANY_TYPE, 
			1, 
			mipLevels);
		
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

	Texture2D::Texture2D() : Texture() {
	
	}
	
	void Texture2D::create(TextureTypeFlags type, Extent extent, uint32_t sampleCount, uint32_t mipLevels) {

		vk::ImageUsageFlags usage = (vk::ImageUsageFlags)type;
		vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor;

		vk::Format format = vk::Format::eUndefined;
		vk::FormatFeatureFlags features;
		if (type & TextureTypeFlagBits::DEPTH_ATTACHMENT) {
			features |= vk::FormatFeatureFlagBits::eDepthStencilAttachment;
			aspect = vk::ImageAspectFlagBits::eDepth;
			format = m_pCore->getDefaultDepthFormat();
		}
		else {
			if (type & TextureTypeFlagBits::SAMPLED) {
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

			format = m_pCore->getFormat(
				FormatPrecisionFlagBits::ANY_PRECISION,
				FormatDimensionFlagBits::ANY_DIMENSION,
				FormatTypeFlagBits::ANY_TYPE,
				features,
				vk::ImageTiling::eOptimal);
		}

		DEBUG_LOG_INFO("Created 2D texture\nExtent: { w:", extent.width, " h:", extent.height, "}\nFormat: ", vk::to_string(format), "\nSampleCount: ", sampleCount);
		m_pImage = m_pCore->createImage2D(
			extent,
			format,
			usage,
			(vk::SampleCountFlagBits)sampleCount,
			mipLevels,
			1
		);


		m_view = ResourceManager::get().insert<vk::ImageView>(m_pCore->createImageView(
			vk::ImageViewType::e2D,
			m_pImage->image,
			format,
			aspect,
			mipLevels,
			0,
			1,
			0
		));

	}

	void Texture2D::create(TextureTypeFlags type, Extent extent, FormatPrecisionFlags precisions, FormatDimensionFlags dimensions, FormatTypeFlags types, uint32_t sampleCount, uint32_t mipLevels) {
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

		DEBUG_LOG_INFO("Created 2D texture\nExtent: { w:", extent.width, " h:", extent.height, "}\nFormat: ", vk::to_string(format), "\nSampleCount: ", sampleCount);
		m_pImage = m_pCore->createImage2D(
			extent,
			format,
			usage,
			(vk::SampleCountFlagBits)sampleCount,
			mipLevels,
			1
		);

		m_view = ResourceManager::get().insert<vk::ImageView>(m_pCore->createImageView(
			vk::ImageViewType::e2D,
			m_pImage->image,
			format,
			aspect,
			mipLevels,
			0,
			1,
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
			1,
			0,
			1,
			0
		));

	}

	void TextureCube::create(TextureTypeFlags type, Extent extent, uint32_t sampleCount, uint32_t mipLevels) {

		vk::ImageUsageFlags usage = (vk::ImageUsageFlags)type;
		vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor;

		if (mipLevels > 1) {
			usage |= vk::ImageUsageFlagBits::eTransferSrc;
		}

		vk::Format format = m_pCore->getDefaultColorFormat();

		m_pImage = m_pCore->createImage2D(
			extent,
			format,
			usage,
			(vk::SampleCountFlagBits)sampleCount,
			mipLevels,
			6,
			vk::ImageCreateFlagBits::eCubeCompatible
		);


		m_view = ResourceManager::get().insert<vk::ImageView>(m_pCore->createImageView(
			vk::ImageViewType::eCube,
			m_pImage->image,
			format,
			aspect,
			mipLevels,
			0,
			6,
			0
		));
		DEBUG_LOG_INFO("Created Cube texture\nExtent: { w:", extent.width, " h:", extent.height, "}\nFormat:", vk::to_string(format), "\nSampleCount:", sampleCount);

	}

	TextureCube::TextureCube(VulkanCore* pCore, const Image& image, bool generateMipmaps) : Texture(pCore) {
		m_type = TextureTypeFlagBits::SAMPLED | TextureTypeFlagBits::TRANSFER_DST;

		uint32_t mipLevels = 1;
		if (generateMipmaps) {
			mipLevels = image.calculateMipLevelCount();
			m_type |= TextureTypeFlagBits::TRANSFER_SRC;
		}

		sa::Extent subExtent = { image.getWidth() / 4, image.getHeight() / 3 };
		create(m_type, subExtent, 1, mipLevels);

		
		m_pStagingBuffer = m_pCore->createBuffer(
			vk::BufferUsageFlagBits::eTransferSrc,
			VMA_MEMORY_USAGE_AUTO,
			VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
			subExtent.width * subExtent.height * image.getChannelCount() * 6,
			nullptr);
		
		// fill staging buffer
		sa::Offset offsets[6] {
			{ 2 * subExtent.width, subExtent.height }, //right
			{ 0, subExtent.height }, //left
			{ subExtent.width, 0 }, // top
			{ subExtent.width, 2 * subExtent.height }, // bottom
			{ subExtent.width, subExtent.height }, // front
			{ 3 * subExtent.width, subExtent.height } // back
		};
		uint32_t size = 0;
		for (int i = 0; i < 6; i++) {
			size += image.getPixelSegment(subExtent, offsets[i], (unsigned char*)m_pStagingBuffer->mappedData + size);
		}
		
		// transfer data
		DataTransfer transfer{
			.type = DataTransfer::Type::BUFFER_TO_IMAGE,
			.srcBuffer = m_pStagingBuffer,
			.dstImage = m_pImage,
		};
		Renderer::get().queueTransfer(transfer);
	}

	TextureCube::TextureCube(VulkanCore* pCore, const std::vector<Image>& images, bool generateMipmaps) : Texture(pCore) {
		m_type = TextureTypeFlagBits::SAMPLED | TextureTypeFlagBits::TRANSFER_DST;
		if (images.size() != 6)
			throw std::runtime_error("Must contain 6 images");

		uint32_t mipLevels = 1;
		if (generateMipmaps) {
			mipLevels = images[0].calculateMipLevelCount();
			m_type |= TextureTypeFlagBits::TRANSFER_SRC;
		}

		create(m_type, images[0].getExtent(), 1, mipLevels);

		size_t layerSize = images[0].getWidth() * images[0].getHeight() * images[0].getChannelCount();
		m_pStagingBuffer = m_pCore->createBuffer(
			vk::BufferUsageFlagBits::eTransferSrc,
			VMA_MEMORY_USAGE_AUTO,
			VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
			layerSize * 6,
			nullptr);

		for (int i = 0; i < 6; i++) {
			memcpy((char*)m_pStagingBuffer->mappedData + layerSize * i, images[i].getPixels(), layerSize);
		}

		// transfer data
		DataTransfer transfer{
			.type = DataTransfer::Type::BUFFER_TO_IMAGE,
			.srcBuffer = m_pStagingBuffer,
			.dstImage = m_pImage,
		};
		Renderer::get().queueTransfer(transfer);
	}

	TextureCube::TextureCube() : Texture() {
	
	}
	
	void Texture3D::create(TextureTypeFlags type, Extent3D extent, uint32_t sampleCount, uint32_t mipLevels, FormatPrecisionFlags formatPercisions, FormatDimensionFlags formatDimensions, FormatTypeFlags formatTypes) {

		vk::Format format = vk::Format::eUndefined;
		vk::FormatFeatureFlags features;
		if (type & TextureTypeFlagBits::DEPTH_ATTACHMENT) {
			features |= vk::FormatFeatureFlagBits::eDepthStencilAttachment;
		}
		if (type & TextureTypeFlagBits::SAMPLED) {
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

		format = m_pCore->getFormat(
			formatPercisions, formatDimensions, formatTypes, 
			features, 
			vk::ImageTiling::eOptimal);
		if (format == vk::Format::eUndefined) {
			throw std::runtime_error("No supported image format found");
		}

		m_pImage = m_pCore->createImage3D(
			extent, 
			format, 
			(vk::ImageUsageFlags)type, 
			(vk::SampleCountFlagBits)sampleCount, 
			1, 
			1);
		

		m_view = ResourceManager::get().insert<vk::ImageView>(m_pCore->createImageView(
			vk::ImageViewType::e3D,
			m_pImage->image,
			format,
			vk::ImageAspectFlagBits::eColor,
			mipLevels,
			0,
			1,
			0
		));

		DEBUG_LOG_INFO("Created 3D texture\nExtent: { w:", extent.width, " h:", extent.height, " d:", extent.depth, " }\nFormat:", vk::to_string(format), "\nSampleCount:", sampleCount);
	}

	Texture3D::Texture3D(VulkanCore* pCore, TextureTypeFlags type, Extent3D extent, uint32_t sampleCount, uint32_t mipLevels, FormatPrecisionFlags formatPercisions, FormatDimensionFlags formatDimensions, FormatTypeFlags formatTypes) : Texture(pCore) {
		m_type = type;
		create(type, extent, sampleCount, mipLevels, formatPercisions, formatDimensions, formatTypes);
	}

	Texture3D::Texture3D() : Texture() {
	
	}

}