#include "pch.h"
#include "Resources/DeviceMemoryManager.hpp"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

namespace sa {

	vk::Format DeviceMemoryManager::getSupportedDepthFormat() {
		std::vector<vk::Format> formats = {
			vk::Format::eD24UnormS8Uint,
			vk::Format::eD16Unorm,			
			vk::Format::eD16UnormS8Uint,
			vk::Format::eD32Sfloat,
			vk::Format::eD32SfloatS8Uint
		};
		vk::Format f = getFormat(
			formats,
			vk::FormatFeatureFlagBits::eDepthStencilAttachment,
			vk::ImageTiling::eOptimal
		);
		if (f == vk::Format::eUndefined) {
			throw std::runtime_error("Could not find supported depth format");
		}
		return f;
	}

	vk::Format DeviceMemoryManager::getFormat(const std::vector<vk::Format>& candidates, vk::FormatFeatureFlagBits features, vk::ImageTiling tilling) {
		for (vk::Format format : candidates) {
			vk::FormatProperties properties = m_physicalDevice.getFormatProperties(format);
			if (tilling == vk::ImageTiling::eOptimal &&
				(properties.optimalTilingFeatures & features) == features)
			{
				return format;
			}
			else if (tilling == vk::ImageTiling::eLinear &&
				(properties.linearTilingFeatures & features) == features)
			{
				return format;
			}
		}
		return vk::Format::eUndefined;
	}
	

	void DeviceMemoryManager::create(vk::Instance instance, vk::Device device, vk::PhysicalDevice physicalDevice, uint32_t apiVersion, const std::vector<uint32_t>& graphicsQueueFamilyIndices, const std::vector<uint32_t>& computeQueueFamilyIndices) {

		m_instance = instance;
		m_physicalDevice = physicalDevice;
		m_device = device;

		m_allocatorInfo.device = device;
		m_allocatorInfo.instance = instance;
		m_allocatorInfo.physicalDevice = physicalDevice;
		m_allocatorInfo.vulkanApiVersion = apiVersion;

		vmaCreateAllocator(&m_allocatorInfo, &m_allocator);

		m_graphicsQueueFamilyIndices = graphicsQueueFamilyIndices;
		m_computeQueueFamilyIndices = computeQueueFamilyIndices;
	}

	void DeviceMemoryManager::destroy() {
		m_device.waitIdle();

		for (auto& buffer : m_buffers) {
			vmaDestroyBuffer(m_allocator, buffer->buffer, buffer->allocation);
			delete buffer;
		}

		for (auto& image : m_images) {
			vmaDestroyImage(m_allocator, image->image, image->allocation);
			delete image;
		}

		vmaDestroyAllocator(m_allocator);
	}


	DeviceBuffer* DeviceMemoryManager::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags allocationFlags, void* initialData) {
		m_buffers.push_back(new DeviceBuffer());
		DeviceBuffer* buffer = m_buffers.back();

		vk::BufferCreateInfo bufferInfo = {
			.size = size,
			.usage = usage,
		};

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = memoryUsage;
		allocInfo.flags = allocationFlags;
		
		VmaAllocationInfo info;
		VkBuffer cbuffer;
		VkBufferCreateInfo cBufferInfo = (VkBufferCreateInfo)bufferInfo;
		vk::Result result = (vk::Result)vmaCreateBuffer(m_allocator, &cBufferInfo, &allocInfo, &cbuffer, &buffer->allocation, &info);
		buffer->buffer = cbuffer;

		checkError(result, "Failed to allocate buffer of size " + std::to_string(size), false);

		buffer->size = size;
		buffer->mappedData = info.pMappedData;
		if (buffer->mappedData != nullptr && initialData != nullptr) {
			memcpy(buffer->mappedData, initialData, buffer->size);
		}

		return buffer;
	}

	DeviceImage* DeviceMemoryManager::createImage(vk::Extent3D extent, uint32_t arrayLayers, vk::Format format,
		vk::ImageType type, vk::ImageLayout initialLayout, uint32_t mipLevels, const std::vector<uint32_t>& queueFamilyIndices,
		vk::SampleCountFlagBits sampleCount, vk::SharingMode sharingMode, vk::ImageTiling tiling, vk::ImageUsageFlags usage,
		VmaMemoryUsage memoryUsage, vk::MemoryPropertyFlags requiredMemoryProperties)
	{
		m_images.push_back(new DeviceImage());
		DeviceImage* image = m_images.back();
		image->format = format;
		image->layout = initialLayout;
		image->sampleCount = sampleCount;
		image->extent = extent;
		image->arrayLayers = arrayLayers;
		image->imageType = type;
		image->mipLevels = mipLevels;
		image->sharingMode = sharingMode;
		image->tiling = tiling;
		image->usage = usage;


		vk::ImageCreateInfo info{
			.imageType = type,
			.format = format,
			.extent = extent,
			.mipLevels = mipLevels,
			.arrayLayers = arrayLayers,
			.samples = sampleCount,
			.tiling = tiling,
			.usage = usage,
			.sharingMode = sharingMode,
			.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size()),
			.pQueueFamilyIndices = queueFamilyIndices.data(),
			.initialLayout = initialLayout,
		};

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = memoryUsage;
		allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
		allocInfo.requiredFlags = (VkMemoryPropertyFlags)requiredMemoryProperties;
		
		VkImageCreateInfo cinfo = (VkImageCreateInfo)info;

		VkImage cimage;
		sa::checkError(
			(vk::Result)vmaCreateImage(m_allocator, &cinfo, &allocInfo, &cimage, &image->allocation, nullptr),
			"Failed to create Image",
			false
		);
		image->image = cimage;

		return image;
	}

	DeviceImage* DeviceMemoryManager::createDepthAttachmentTexture2D(vk::Extent2D extent, vk::SampleCountFlagBits sampleCount,
		uint32_t mipLevels, uint32_t arrayLayers) 
	{
		auto image = createImage(
			{ extent.width, extent.height, 1 },
			arrayLayers,
			getSupportedDepthFormat(),
			vk::ImageType::e2D,
			vk::ImageLayout::eUndefined,
			mipLevels,
			m_graphicsQueueFamilyIndices,
			sampleCount,
			vk::SharingMode::eExclusive,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eDepthStencilAttachment,
			VMA_MEMORY_USAGE_GPU_ONLY,
			vk::MemoryPropertyFlagBits::eDeviceLocal
		);

		return image;
	}

	DeviceImage* DeviceMemoryManager::createColorTexture2D(vk::Extent2D extent, vk::ImageUsageFlags usage, vk::Format format, vk::SampleCountFlagBits sampleCount,
		uint32_t mipLevels, uint32_t arrayLayers) 
	{
		auto image = createImage(
			{ extent.width, extent.height, 1 },
			arrayLayers,
			format,
			vk::ImageType::e2D,
			vk::ImageLayout::eUndefined,
			mipLevels,
			m_graphicsQueueFamilyIndices,
			sampleCount,
			vk::SharingMode::eExclusive,
			vk::ImageTiling::eOptimal,
			usage,
			VMA_MEMORY_USAGE_GPU_ONLY,
			vk::MemoryPropertyFlagBits::eDeviceLocal
		);

		return image;
	}

	DeviceImage* DeviceMemoryManager::createTexture3D(vk::Extent3D extent, vk::ImageUsageFlags usage, vk::SampleCountFlagBits sampleCount, vk::Format format, uint32_t mipLevels, uint32_t arrayLayers) {
		auto image = createImage(
			{ extent.width, extent.height, 1 },
			arrayLayers,
			format,
			vk::ImageType::e3D,
			vk::ImageLayout::eUndefined,
			mipLevels,
			m_graphicsQueueFamilyIndices,
			sampleCount,
			vk::SharingMode::eExclusive,
			vk::ImageTiling::eOptimal,
			usage,
			VMA_MEMORY_USAGE_GPU_ONLY,
			vk::MemoryPropertyFlagBits::eDeviceLocal
		);

		return image;
	}
	
	void DeviceMemoryManager::destroyBuffer(DeviceBuffer* buffer) {
		vmaDestroyBuffer(m_allocator, buffer->buffer, buffer->allocation);
		m_buffers.erase(std::find(m_buffers.begin(), m_buffers.end(), buffer));
		delete buffer;
	}

	void DeviceMemoryManager::destroyImage(DeviceImage* texture) {
		vmaDestroyImage(m_allocator, texture->image, texture->allocation);
		m_images.erase(std::find(m_images.begin(), m_images.end(), texture));
		delete texture;
	}
}