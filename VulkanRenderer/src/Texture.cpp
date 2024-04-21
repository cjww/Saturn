#include "pch.h"
#include "Resources/Texture.hpp"
#include "internal/Swapchain.hpp"

#include "internal/VulkanCore.hpp"
#include "Renderer.hpp"

#include "internal/DeviceMemoryManager.hpp"

namespace sa {


	Texture::Texture(VulkanCore* pCore)
		: m_pCore(pCore)
		, m_pImage(nullptr)
		, m_pStagingBuffer(nullptr)
		, m_usage(0)
		, m_pDataTransfer(nullptr)
	{
	}

	Texture::Texture(ImageView imageView, TextureUsageFlags usage, TextureType type) : Texture() {
		m_view = imageView;
		m_usage = usage;
		m_type = type;
	}


	ImageView Texture::createImageView(TextureType viewType, uint32_t mipLevels, uint32_t baseMipLevel, uint32_t layers, uint32_t baseArrayLevel) {
		vk::ImageViewType type = static_cast<vk::ImageViewType>(viewType);

		vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor;
		if (VulkanCore::IsDepthFormat(m_pImage->format)) {
			aspect = vk::ImageAspectFlagBits::eDepth;
			/*
			if (VulkanCore::HasStencilComponent(m_pImage->format)) {
				aspect |= vk::ImageAspectFlagBits::eStencil;
			}
			*/
		}

		uint32_t aspectInt = static_cast<uint32_t>(aspect);
		ImageView view;
		view.create(
			m_pImage,
			&type,
			&aspectInt,
			mipLevels,
			baseMipLevel,
			layers,
			baseArrayLevel);
		return view;
	}

	Texture::Texture() : Texture(Renderer::Get().m_pCore.get()) {

	}

	void Texture::create2D(TextureType type, TextureUsageFlags usageFlags, Extent extent, Format format, uint32_t mipLevels, uint32_t arrayLayers, uint32_t samples, uint32_t imageCreateFlags) {
		m_usage = usageFlags;
		m_type = type;
		vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor;

		if (m_usage & TextureUsageFlagBits::DEPTH_ATTACHMENT) {
			aspect = vk::ImageAspectFlagBits::eDepth;
			if (format == Format::UNDEFINED) {
				format = Renderer::Get().getDefaultDepthFormat();
			}
		}

		extent = { std::max(extent.width, 1U), std::max(extent.height, 1U) };
		mipLevels = std::min(mipLevels, (uint32_t)floor(log2(std::max(extent.width, extent.height))) + 1);

		if (format == Format::UNDEFINED) {
			format = Renderer::Get().selectFormat(m_usage);
		}

		m_pImage = m_pCore->createImage2D(
			extent,
			(vk::Format)format,
			(vk::ImageUsageFlags)m_usage,
			(vk::SampleCountFlagBits)samples,
			mipLevels,
			arrayLayers,
			(vk::ImageCreateFlags)imageCreateFlags
		);

		m_view = createImageView(type, mipLevels, 0, arrayLayers, 0);

	}

	void Texture::create2D(TextureUsageFlags usageFlags, Extent extent, Format format, uint32_t mipLevels, uint32_t arrayLayers, uint32_t samples) {
		create2D((arrayLayers > 1) ? TextureType::TEXTURE_TYPE_2D_ARRAY : TextureType::TEXTURE_TYPE_2D, usageFlags, extent, format, mipLevels, arrayLayers, samples, 0);
	}

	void Texture::create2D(const Image& image, bool generateMipmaps) {
		TextureUsageFlags usage = TextureUsageFlagBits::SAMPLED | TextureUsageFlagBits::TRANSFER_DST;

		uint32_t mipLevels = 1;
		if (generateMipmaps) {
			mipLevels = image.calculateMipLevelCount();
			usage |= TextureUsageFlagBits::TRANSFER_SRC;
		}

		Format format = Renderer::Get().selectFormat(
			sa::FormatPrecisionFlagBits::e8Bit,
			sa::FormatDimensionFlagBits::e4,
			sa::FormatTypeFlagBits::ANY_TYPE,
			usage);

		create2D(
			usage,
			image.getExtent(),
			format,
			mipLevels,
			1,
			1);

		m_pStagingBuffer = m_pCore->createBuffer(
			vk::BufferUsageFlagBits::eTransferSrc,
			VMA_MEMORY_USAGE_AUTO,
			VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
			image.getWidth() * image.getHeight() * image.getChannelCount(),
			image.getPixels());
		// transfer data
		DataTransfer transfer{
			.type = DataTransfer::Type::BUFFER_TO_IMAGE,
			.srcBuffer = m_pStagingBuffer,
			.dstImage = m_pImage,
		};
		m_pDataTransfer = Renderer::Get().queueTransfer(transfer);
	}


	void Texture::createCube(TextureUsageFlags usageFlags, Extent extent, Format format, uint32_t mipLevels, uint32_t samples) {
		create2D(TextureType::TEXTURE_TYPE_CUBE, usageFlags, extent, format, mipLevels, 6, samples, static_cast<uint32_t>(vk::ImageCreateFlagBits::eCubeCompatible));
	}

	void Texture::createCube(const Image& image, bool generateMipmaps) {
		TextureUsageFlags usage = TextureUsageFlagBits::SAMPLED | TextureUsageFlagBits::TRANSFER_DST;

		uint32_t mipLevels = 1;
		if (generateMipmaps) {
			mipLevels = image.calculateMipLevelCount();
			usage |= TextureUsageFlagBits::TRANSFER_SRC;
		}

		sa::Extent subExtent = { image.getWidth() / 4, image.getHeight() / 3 };

		Format format = Renderer::Get().selectFormat(
			sa::FormatPrecisionFlagBits::e8Bit,
			sa::FormatDimensionFlagBits::e4,
			sa::FormatTypeFlagBits::ANY_TYPE,
			usage);

		create2D(
			TextureType::TEXTURE_TYPE_CUBE, 
			usage, 
			subExtent, 
			format, 
			mipLevels, 
			6, 
			1, 
			static_cast<uint32_t>(vk::ImageCreateFlagBits::eCubeCompatible));


		m_pStagingBuffer = m_pCore->createBuffer(
			vk::BufferUsageFlagBits::eTransferSrc,
			VMA_MEMORY_USAGE_AUTO,
			VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
			subExtent.width * subExtent.height * image.getChannelCount() * 6,
			nullptr);

		// fill staging buffer
		sa::Offset offsets[6]{
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
		m_pDataTransfer = Renderer::Get().queueTransfer(transfer);
	}

	void Texture::createCube(const std::vector<Image>& images, bool generateMipmaps) {
		TextureUsageFlags usage = TextureUsageFlagBits::SAMPLED | TextureUsageFlagBits::TRANSFER_DST;
		if (images.size() != 6)
			throw std::runtime_error("Must contain 6 images");

		uint32_t mipLevels = 1;
		if (generateMipmaps) {
			mipLevels = images[0].calculateMipLevelCount();
			usage |= TextureUsageFlagBits::TRANSFER_SRC;
		}

		Format format = Renderer::Get().selectFormat(
			sa::FormatPrecisionFlagBits::e8Bit,
			sa::FormatDimensionFlagBits::e4,
			sa::FormatTypeFlagBits::ANY_TYPE,
			usage);

		create2D(
			TextureType::TEXTURE_TYPE_CUBE,
			usage,
			images[0].getExtent(),
			format,
			mipLevels,
			6,
			1,
			static_cast<uint32_t>(vk::ImageCreateFlagBits::eCubeCompatible));

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
		m_pDataTransfer = Renderer::Get().queueTransfer(transfer);
	}


	void Texture::create3D(TextureUsageFlags usageFlags, Extent3D extent, Format format, uint32_t mipLevels, uint32_t arrayLayers, uint32_t samples) {
		m_usage = usageFlags;
		m_type = TextureType::TEXTURE_TYPE_3D;

		extent = { std::max(extent.width, 1U), std::max(extent.height, 1U), std::max(extent.depth, 1U) };
		mipLevels = std::min(mipLevels, (uint32_t)floor(log2(std::max(extent.width, std::max(extent.height, extent.depth)))) + 1);

		if (format == Format::UNDEFINED) {
			format = Renderer::Get().selectFormat(m_usage);
		}

		m_pImage = m_pCore->createImage3D(
			extent,
			(vk::Format)format,
			(vk::ImageUsageFlags)usageFlags,
			(vk::SampleCountFlagBits)samples,
			mipLevels,
			arrayLayers);


		m_view = createImageView(TextureType::TEXTURE_TYPE_3D, mipLevels, 0, arrayLayers, 0);

		//SA_DEBUG_LOG_INFO("Created 3D texture\nExtent: { w:", extent.width, " h:", extent.height, " d:", extent.depth, " }\nFormat:", vk::to_string(format), "\nSampleCount:", sampleCount);
	}

	std::vector<Texture> Texture::createMipLevelTextures() {
		uint32_t count = getMipLevelCount();
		std::vector<Texture> textures(count);
		createMipLevelTextures(&count, textures.data());
		return std::move(textures);
	}

	void Texture::createMipLevelTextures(uint32_t* count, Texture* pTextures) {
		assert(m_pImage && "This is not a full texture");

		*count = getMipLevelCount();
		if (!pTextures) {
			return;
		}

		for (uint32_t i = 0; i < *count; i++) {
			pTextures[i] = Texture(createImageView(m_type, 1, i, 1, 0), m_usage, m_type);
		}
	}


	std::vector<Texture> Texture::createArrayLayerTextures() {
		uint32_t count = getArrayLayerCount();
		std::vector<Texture> textures(count);
		createArrayLayerTextures(&count, textures.data());
		return std::move(textures);
	}

	void Texture::createArrayLayerTextures(uint32_t* count, Texture* pTextures) {
		assert(m_pImage && "This is not a full texture");

		*count = getArrayLayerCount();
		if (!pTextures) {
			return;
		}

		TextureType viewType = m_type;
		switch (m_type) {
			case TextureType::TEXTURE_TYPE_1D_ARRAY:
				viewType = TextureType::TEXTURE_TYPE_1D;
				break;
			case TextureType::TEXTURE_TYPE_2D_ARRAY:
				viewType = TextureType::TEXTURE_TYPE_2D;
				break;
			case TextureType::TEXTURE_TYPE_CUBE_ARRAY:
			case TextureType::TEXTURE_TYPE_CUBE:
				viewType = TextureType::TEXTURE_TYPE_2D;
		}

		for (uint32_t i = 0; i < *count; i++) {
			pTextures[i] = Texture(createImageView(viewType, 1, 0, 1, i), m_usage, viewType);
		}
	}

	void* Texture::getImageHandle() const {
		return m_pImage->image;
	}

	Extent Texture::getExtent() const {
		return { m_pImage->extent.width, m_pImage->extent.height };
	}

	Extent3D Texture::getExtent3D() const {
		return { m_pImage->extent.width, m_pImage->extent.height, m_pImage->extent.depth };
	}

	uint32_t Texture::getDepth() const {
		return m_pImage->extent.depth;
	}

	const ImageView& Texture::getView() const {
		return m_view;
	}

	TextureUsageFlags Texture::getUsageFlags() const {
		return m_usage;
	}

	TextureType Texture::getTextureType() const {
		return m_type;
	}

	uint32_t Texture::getArrayLayerCount() const {
		return m_pImage->arrayLayers;
	}

	uint32_t Texture::getMipLevelCount() const {
		return m_pImage->mipLevels;
	}

	Format Texture::getFormat() const {
		return static_cast<Format>(m_pImage->format);
	}

	bool Texture::isValid() const {
		return isValidImage() && isValidView();
	}

	bool Texture::isValidImage() const {
		return m_pImage != nullptr;
	}

	bool Texture::isValidView() const {
		return m_view.isValid();
	}

	void Texture::destroy() {
		m_pCore->getDevice().waitIdle();
		if (m_pDataTransfer) {
			if (sa::Renderer::Get().cancelTransfer(m_pDataTransfer)) {
				SA_DEBUG_LOG_INFO("Canceled image transfer");
			}
		}
		if (isValidView()) {
			m_view.destroy();
		}
		if (isValidImage()) {
			if (m_pStagingBuffer)
				m_pCore->destroyBuffer(m_pStagingBuffer);

			if (m_pImage) {
				m_pCore->destroyImage(m_pImage);
				m_pImage = nullptr;
			}
		}
	}

	bool Texture::operator==(const Texture& other) {
		return m_view == other.m_view;
	}

	bool Texture::operator!=(const Texture& other) {
		return m_view != other.m_view;
	}
}