#include "DataManager.hpp"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "vulkan_base.hpp"

namespace NAME_SPACE {

	VkFormat DataManager::getSupportedDepthFormat() {
		std::vector<VkFormat> formats = {
			VK_FORMAT_D24_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM,
			VK_FORMAT_D16_UNORM_S8_UINT,
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D32_SFLOAT_S8_UINT
		};
		VkFormat f = getFormat(formats, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);
		if (f == VK_FORMAT_UNDEFINED) {
			throw std::runtime_error("Could not find supported depth format");
		}
		return f;
	}

	VkFormat DataManager::getFormat(const std::vector<VkFormat>& candidates, VkFormatFeatureFlagBits features, VkImageTiling tilling) {
		for (VkFormat format : candidates) {
			VkFormatProperties properties = {};
			vkGetPhysicalDeviceFormatProperties(m_allocatorInfo.physicalDevice, format, &properties);
			if (tilling == VK_IMAGE_TILING_OPTIMAL &&
				(properties.optimalTilingFeatures & features) == features)
			{
				return format;
			}
			else if (tilling == VK_IMAGE_TILING_LINEAR &&
				(properties.linearTilingFeatures & features) == features)
			{
				return format;
			}
		}
		return VK_FORMAT_UNDEFINED;
	}
	DataManager::DataManager(VkInstance instance, VkDevice device, VkPhysicalDevice physicalDevice, uint32_t apiVersion,
		const std::vector<uint32_t>& graphicsQueueFamilyIndices, const std::vector<uint32_t>& computeQueueFamilyIndices) {

		m_allocatorInfo.device = device;
		m_allocatorInfo.instance = instance;
		m_allocatorInfo.physicalDevice = physicalDevice;
		m_allocatorInfo.vulkanApiVersion = apiVersion;

		vmaCreateAllocator(&m_allocatorInfo, &m_allocator);

		m_graphicsQueueFamilyIndices = graphicsQueueFamilyIndices;
		m_computeQueueFamilyIndices = computeQueueFamilyIndices;

	}

	DataManager::~DataManager() {
		vkDeviceWaitIdle(m_allocatorInfo.device);

		for (auto& buffer : m_buffers) {
			vmaDestroyBuffer(m_allocator, buffer->buffer, buffer->allocation);
			delete buffer;
		}

		for (auto& texture : m_textures) {
			vmaDestroyImage(m_allocator, texture->image, texture->allocation);
			if (texture->view != VK_NULL_HANDLE) {
				vkDestroyImageView(m_allocatorInfo.device, texture->view, nullptr);
			}
			delete texture;
		}

		vmaDestroyAllocator(m_allocator);
	}

	Buffer* DataManager::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, void* initialData) {
		m_buffers.push_back(new Buffer());
		Buffer* buffer = m_buffers.back();

		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = memoryUsage;
		allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
		
		VmaAllocationInfo info;
		vmaCreateBuffer(m_allocator, &bufferInfo, &allocInfo, &buffer->buffer, &buffer->allocation, &info);
		
		buffer->size = size;
		buffer->mappedData = info.pMappedData;
		if (buffer->mappedData != nullptr && initialData != nullptr) {
			memcpy(buffer->mappedData, initialData, buffer->size);
		}

		return buffer;
	}

	void DataManager::createImageView(VkImageViewType type, Texture* image,
		VkImageAspectFlags aspectMask, uint32_t baseMiplevel, uint32_t baseArrayLevel)
	{
		VkImageView view;
		createImageView(view, type, image->image, image->format, aspectMask, baseMiplevel, baseArrayLevel);
		image->view = view;
	}

	void DataManager::createImageView(VkImageView& view, VkImageViewType type, VkImage image, VkFormat format,
		VkImageAspectFlags aspectMask, uint32_t baseMiplevel, uint32_t baseArrayLevel)
	{
		VkImageViewCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

		info.viewType = type;
		info.format = format;
		info.image = image;
		info.subresourceRange.aspectMask = aspectMask;
		info.subresourceRange.baseMipLevel = baseMiplevel;
		info.subresourceRange.levelCount = 1;
		info.subresourceRange.baseArrayLayer = baseArrayLevel;
		info.subresourceRange.layerCount = 1;
		info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		if (vkCreateImageView(m_allocatorInfo.device, &info, nullptr, &view) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create image view");
		}
	}

	Texture* DataManager::createImage(VkExtent3D extent, uint32_t arrayLayers, VkFormat format,
		VkImageType type, VkImageLayout initialLayout, uint32_t mipLevels, const std::vector<uint32_t>& queueFamilyIndices,
		VkSampleCountFlagBits sampleCount, VkSharingMode sharingMode, VkImageTiling tiling, VkImageUsageFlags usage,
		VmaMemoryUsage memoryUsage, VkMemoryPropertyFlags requiredMemoryProperties)
	{
		m_textures.push_back(new Texture());
		Texture* image = m_textures.back();
		image->format = format;
		image->layout = initialLayout;
		image->sampleCount = sampleCount;
		image->extent = extent;


		VkImageCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		info.pNext = 0;
		info.flags = 0;
		info.arrayLayers = arrayLayers;
		info.extent = extent;
		info.format = format;
		info.imageType = type;
		info.initialLayout = initialLayout;
		info.mipLevels = mipLevels;
		info.pQueueFamilyIndices = queueFamilyIndices.data();
		info.queueFamilyIndexCount = queueFamilyIndices.size();
		info.samples = sampleCount;
		info.sharingMode = sharingMode;
		info.tiling = tiling;
		info.usage = usage;

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = memoryUsage;
		allocInfo.requiredFlags = requiredMemoryProperties;
		
		vbl::printError(
			vmaCreateImage(m_allocator, &info, &allocInfo, &image->image, &image->allocation, nullptr),
			"Failed to create Image"
		);

		return image;
	}

	Texture* DataManager::createDepthAttachmentTexture2D(VkExtent2D extent, VkSampleCountFlagBits sampleCount,
		uint32_t mipLevels, uint32_t arrayLayers) 
	{
		auto image = createImage(
			{ extent.width, extent.height, 1 },
			arrayLayers,
			getSupportedDepthFormat(),
			VK_IMAGE_TYPE_2D,
			VK_IMAGE_LAYOUT_UNDEFINED,
			mipLevels,
			m_graphicsQueueFamilyIndices,
			sampleCount,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		createImageView(VK_IMAGE_VIEW_TYPE_2D, image, VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0);

		return image;
	}

	Texture* DataManager::createColorTexture2D(VkExtent2D extent, VkImageUsageFlags usage, VkSampleCountFlagBits sampleCount,
		uint32_t mipLevels, uint32_t arrayLayers) 
	{
		auto image = createImage(
			{ extent.width, extent.height, 1 },
			arrayLayers,
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_TYPE_2D,
			VK_IMAGE_LAYOUT_UNDEFINED,
			mipLevels,
			m_graphicsQueueFamilyIndices,
			sampleCount,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_IMAGE_TILING_OPTIMAL,
			usage,
			VMA_MEMORY_USAGE_GPU_ONLY,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		createImageView(VK_IMAGE_VIEW_TYPE_2D, image, VK_IMAGE_ASPECT_COLOR_BIT, 0, 0);

		return image;
	}
	Texture* DataManager::createTexture3D(VkExtent3D extent, VkImageUsageFlags usage, VkSampleCountFlagBits sampleCount, VkFormat format, uint32_t mipLevels, uint32_t arrayLayers)
	{
		auto image = createImage(
			extent,
			arrayLayers,
			format,
			VK_IMAGE_TYPE_3D,
			VK_IMAGE_LAYOUT_UNDEFINED,
			mipLevels,
			m_graphicsQueueFamilyIndices,
			sampleCount,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_IMAGE_TILING_OPTIMAL,
			usage,
			VMA_MEMORY_USAGE_GPU_ONLY,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		createImageView(VK_IMAGE_VIEW_TYPE_3D, image, VK_IMAGE_ASPECT_COLOR_BIT, 0, 0);
		return image;
	}
	void DataManager::destroyBuffer(Buffer* buffer) {
		vmaDestroyBuffer(m_allocator, buffer->buffer, buffer->allocation);
		m_buffers.erase(std::find(m_buffers.begin(), m_buffers.end(), buffer));
		delete buffer;
	}
	void DataManager::destroyImage(Texture* texture) {
		vmaDestroyImage(m_allocator, texture->image, texture->allocation);
		if (texture->view != VK_NULL_HANDLE) {
			vkDestroyImageView(m_allocatorInfo.device, texture->view, nullptr);
		}
		m_textures.erase(std::find(m_textures.begin(), m_textures.end(), texture));
		delete texture;
	}
}