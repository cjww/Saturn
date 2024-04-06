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
		, m_view(NULL_RESOURCE)
		, m_type(0)
		, m_pDataTransfer(nullptr)
	{
	}

	ResourceID Texture::createImageView(TextureViewType viewType, uint32_t mipLevels, uint32_t baseMipLevel, uint32_t layers, uint32_t baseArrayLevel) {
		vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor;
		if (m_type & TextureTypeFlagBits::DEPTH_ATTACHMENT) {
			aspect = vk::ImageAspectFlagBits::eDepth;
		}
		return ResourceManager::get().insert<vk::ImageView>(m_pCore->createImageView(
			(vk::ImageViewType)viewType,
			m_pImage->image,
			m_pImage->format,
			aspect,
			mipLevels,
			baseMipLevel,
			layers,
			baseArrayLevel
		));
	}

	Texture::Texture() : Texture(Renderer::get().m_pCore.get()) {

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

	uint32_t Texture::getArrayLayerCount() const {
		return m_pImage->arrayLayers;
	}

	uint32_t Texture::getMipLevelCount() const {
		return m_pImage->mipLevels;
	}

	bool Texture::isValid() const {
		return isValidImage() && isValidView();
	}

	bool Texture::isValidImage() const {
		return m_pImage != nullptr;
	}

	bool Texture::isValidView() const {
		return m_view != NULL_RESOURCE;
	}

	bool Texture::isSampleReady() const {
		return isValidImage() && 
			(m_pImage->layout == vk::ImageLayout::eShaderReadOnlyOptimal ||
			m_pImage->layout == vk::ImageLayout::eGeneral);
	}

	void Texture::destroy() {
		m_pCore->getDevice().waitIdle();
		if (m_pDataTransfer) {
			if (sa::Renderer::get().cancelTransfer(m_pDataTransfer)) {
				SA_DEBUG_LOG_INFO("Canceled image transfer");
			}
		}
		if (isValidView()) {
			ResourceManager::get().remove<vk::ImageView>(m_view);
			m_view = NULL_RESOURCE;
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


	Texture2D::Texture2D(TextureTypeFlags type, Extent extent, uint32_t sampleCount, uint32_t mipLevels, uint32_t arrayLayers)
		: Texture()
	{
		m_type = type;
		create(type, extent, sampleCount, mipLevels, arrayLayers);
	}

	Texture2D::Texture2D(TextureTypeFlags type, Extent extent, Format format, uint32_t sampleCount, uint32_t mipLevels, uint32_t arrayLayers) {
		m_type = type;
		create(type, extent, format, sampleCount, mipLevels, arrayLayers);
	}

	Texture2D::Texture2D(TextureTypeFlags type, Extent extent, Swapchain* pSwapchain, uint32_t sampleCount) 
		: Texture()
	{
		m_type = type;
		create(type, extent, pSwapchain, sampleCount);
	}

	Texture2D::Texture2D(const Image& image, bool generateMipmaps)
		: Texture()
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
			Renderer::get().selectFormat(
				sa::FormatPrecisionFlagBits::e8Bit, 
				sa::FormatDimensionFlagBits::e4, 
				sa::FormatTypeFlagBits::ANY_TYPE,
				m_type),
			1, 
			mipLevels,
			1);
		
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
		m_pDataTransfer = Renderer::get().queueTransfer(transfer);
	}

	Texture2D::Texture2D(ResourceID imageView) : Texture() {
		m_view = imageView;
	}

	Texture2D::Texture2D() : Texture() {
	
	}

	std::vector<Texture2D> Texture2D::createMipLevelTextures() {
		assert(m_pImage && "This is not a full texture");
		std::vector<Texture2D> textures(m_pImage->mipLevels);

		for (uint32_t i = 0; i < textures.size(); i++) {
			textures[i] = Texture2D(createImageView(TextureViewType::VIEW_TYPE_2D, 1, i, 1, 0));
			textures[i].m_type = m_type;
		}

		return textures;
	}

	void Texture2D::createMipLevelTextures(uint32_t* count, Texture2D* pTextures) {
		assert(m_pImage && "This is not a full texture");

		*count = getMipLevelCount();
		if (!pTextures) {
			return;
		}

		for (uint32_t i = 0; i < *count; i++) {
			pTextures[i] = Texture2D(createImageView(TextureViewType::VIEW_TYPE_2D, 1, i, 1, 0));
			pTextures[i].m_type = m_type;
		}
	}

	void Texture2D::createArrayLayerTextures(uint32_t* count, Texture2D* pTextures) {
		assert(m_pImage && "This is not a full texture");

		*count = getArrayLayerCount();
		if (!pTextures) {
			return;
		}

		for (uint32_t i = 0; i < *count; i++) {
			pTextures[i] = Texture2D(createImageView(TextureViewType::VIEW_TYPE_2D, 1, 0, 1, i));
			pTextures[i].m_type = m_type;
		}

	}

	std::vector<Texture2D> Texture2D::createArrayLayerTextures() {
		assert(m_pImage && "This is not a full texture");
		std::vector<Texture2D> textures(m_pImage->arrayLayers);

		for (uint32_t i = 0; i < textures.size(); i++) {
			textures[i] = Texture2D(createImageView(TextureViewType::VIEW_TYPE_2D, 1, 0, 1, i));
			textures[i].m_type = m_type;
		}

		return textures;
	}
	
	void Texture2D::create(TextureTypeFlags type, Extent extent, uint32_t sampleCount, uint32_t mipLevels, uint32_t arrayLayers) {

		vk::ImageUsageFlags usage = (vk::ImageUsageFlags)type;
		vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor;

		vk::Format format = vk::Format::eUndefined;
		if (type & TextureTypeFlagBits::DEPTH_ATTACHMENT) {
			aspect = vk::ImageAspectFlagBits::eDepth;// | vk::ImageAspectFlagBits::eStencil;
			format = m_pCore->getDefaultDepthFormat();
		}
		else {
			format = (vk::Format)Renderer::get().selectFormat(type);
		}
		extent = { std::max(extent.width, 1U), std::max(extent.height, 1U) };
		mipLevels = std::min(mipLevels, (uint32_t)floor(log2(std::max(extent.width, extent.height))) + 1);

		//SA_DEBUG_LOG_INFO("Created 2D texture\nExtent: { w:", extent.width, " h:", extent.height, "}\nFormat: ", vk::to_string(format), "\nSampleCount: ", sampleCount);
		m_pImage = m_pCore->createImage2D(
			extent,
			format,
			usage,
			(vk::SampleCountFlagBits)sampleCount,
			mipLevels,
			arrayLayers
		);

		vk::ImageViewType viewType = vk::ImageViewType::e2D;
		if (arrayLayers > 1) {
			viewType = vk::ImageViewType::e2DArray;
		}


		m_view = ResourceManager::get().insert<vk::ImageView>(m_pCore->createImageView(
			viewType,
			m_pImage->image,
			format,
			aspect,
			mipLevels,
			0,
			arrayLayers,
			0
		));

	}

	void Texture2D::create(TextureTypeFlags type, Extent extent, Format format, uint32_t sampleCount, uint32_t mipLevels, uint32_t arrayLayers) {
		vk::ImageUsageFlags usage = (vk::ImageUsageFlags)type;
		vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor;


		if (type & TextureTypeFlagBits::DEPTH_ATTACHMENT) {
			aspect = vk::ImageAspectFlagBits::eDepth;
		}
		extent = { std::max(extent.width, 1U), std::max(extent.height, 1U) };
		mipLevels = std::min(mipLevels, (uint32_t)floor(log2(std::max(extent.width, extent.height))) + 1);

		m_pImage = m_pCore->createImage2D(
			extent,
			(vk::Format)format,
			usage,
			(vk::SampleCountFlagBits)sampleCount,
			mipLevels,
			arrayLayers
		);


		vk::ImageViewType viewType = vk::ImageViewType::e2D;
		if (arrayLayers > 1) {
			viewType = vk::ImageViewType::e2DArray;
		}

		m_view = ResourceManager::get().insert<vk::ImageView>(m_pCore->createImageView(
			viewType,
			m_pImage->image,
			(vk::Format)format,
			aspect,
			mipLevels,
			0,
			arrayLayers,
			0
		));
	}


	void Texture2D::create(TextureTypeFlags type, Extent extent, Swapchain* pSwapchain, uint32_t sampleCount) {

		vk::ImageUsageFlags usage = (vk::ImageUsageFlags)type;
		
		//SA_DEBUG_LOG_INFO("Created 2D texture\nFormat: ", vk::to_string(pSwapchain->getFormat()));
		extent = { std::max(extent.width, 1U), std::max(extent.height, 1U) };

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

	void TextureCube::create(TextureTypeFlags type, Extent extent, uint32_t sampleCount, uint32_t mipLevels, Format format) {

		vk::ImageUsageFlags usage = (vk::ImageUsageFlags)type;
		vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor;

		if (mipLevels > 1) {
			usage |= vk::ImageUsageFlagBits::eTransferSrc;
		}

		extent = { std::max(extent.width, 1U), std::max(extent.height, 1U) };
		mipLevels = std::min(mipLevels, (uint32_t)floor(log2(std::max(extent.width, extent.height))) + 1);

		m_pImage = m_pCore->createImage2D(
			extent,
			(vk::Format)format,
			usage,
			(vk::SampleCountFlagBits)sampleCount,
			mipLevels,
			6,
			vk::ImageCreateFlagBits::eCubeCompatible
		);


		m_view = ResourceManager::get().insert<vk::ImageView>(m_pCore->createImageView(
			vk::ImageViewType::eCube,
			m_pImage->image,
			(vk::Format)format,
			aspect,
			mipLevels,
			0,
			6,
			0
		));
		//SA_DEBUG_LOG_INFO("Created Cube texture\nExtent: { w:", extent.width, " h:", extent.height, "}\nFormat:", vk::to_string(format), "\nSampleCount:", sampleCount);

	}

	TextureCube::TextureCube(const Image& image, bool generateMipmaps, Format format) : Texture() {
		m_type = TextureTypeFlagBits::SAMPLED | TextureTypeFlagBits::TRANSFER_DST;

		uint32_t mipLevels = 1;
		if (generateMipmaps) {
			mipLevels = image.calculateMipLevelCount();
			m_type |= TextureTypeFlagBits::TRANSFER_SRC;
		}

		sa::Extent subExtent = { image.getWidth() / 4, image.getHeight() / 3 };
		create(m_type, subExtent, 1, mipLevels, format);

		
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
		m_pDataTransfer = Renderer::get().queueTransfer(transfer);
	}

	TextureCube::TextureCube(const std::vector<Image>& images, bool generateMipmaps, Format format) : Texture() {
		m_type = TextureTypeFlagBits::SAMPLED | TextureTypeFlagBits::TRANSFER_DST;
		if (images.size() != 6)
			throw std::runtime_error("Must contain 6 images");

		uint32_t mipLevels = 1;
		if (generateMipmaps) {
			mipLevels = images[0].calculateMipLevelCount();
			m_type |= TextureTypeFlagBits::TRANSFER_SRC;
		}

		create(m_type, images[0].getExtent(), 1, mipLevels, format);

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
		m_pDataTransfer = Renderer::get().queueTransfer(transfer);
	}

	TextureCube::TextureCube() : Texture() {
	
	}
	
	void Texture3D::create(TextureTypeFlags type, Extent3D extent, uint32_t sampleCount, uint32_t mipLevels, Format format) {

		extent = { std::max(extent.width, 1U), std::max(extent.height, 1U), std::max(extent.depth, 1U) };
		mipLevels = std::min(mipLevels, (uint32_t)floor(log2(std::max(extent.width, std::max(extent.height, extent.depth)))) + 1);

		m_pImage = m_pCore->createImage3D(
			extent, 
			(vk::Format)format, 
			(vk::ImageUsageFlags)type, 
			(vk::SampleCountFlagBits)sampleCount, 
			1, 
			1);
		

		m_view = ResourceManager::get().insert<vk::ImageView>(m_pCore->createImageView(
			vk::ImageViewType::e3D,
			m_pImage->image,
			(vk::Format)format,
			vk::ImageAspectFlagBits::eColor,
			mipLevels,
			0,
			1,
			0
		));

		//SA_DEBUG_LOG_INFO("Created 3D texture\nExtent: { w:", extent.width, " h:", extent.height, " d:", extent.depth, " }\nFormat:", vk::to_string(format), "\nSampleCount:", sampleCount);
	}

	Texture3D::Texture3D(TextureTypeFlags type, Extent3D extent, uint32_t sampleCount, uint32_t mipLevels, Format format) : Texture() {
		m_type = type;
		create(type, extent, sampleCount, mipLevels, format);
	}

	Texture3D::Texture3D() : Texture() {
	
	}

}