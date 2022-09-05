#include "pch.h"
#include "Resources/DeviceMemoryManager.hpp"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

namespace sa {


	void DeviceMemoryManager::create(vk::Instance instance, vk::Device device, vk::PhysicalDevice physicalDevice, uint32_t apiVersion) {

		m_instance = instance;
		m_physicalDevice = physicalDevice;
		m_device = device;

		m_allocatorInfo.device = device;
		m_allocatorInfo.instance = instance;
		m_allocatorInfo.physicalDevice = physicalDevice;
		m_allocatorInfo.vulkanApiVersion = apiVersion;
		m_allocatorInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;

		vmaCreateAllocator(&m_allocatorInfo, &m_allocator);
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
		m_memoryMutex.lock();

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

		checkError(result, "Failed to allocate buffer of size " + std::to_string(size));

		buffer->size = size;
		buffer->mappedData = info.pMappedData;
		if (buffer->mappedData != nullptr && initialData != nullptr) {
			memcpy(buffer->mappedData, initialData, buffer->size);
		}
		m_memoryMutex.unlock();
		return buffer;
	}

	DeviceImage* DeviceMemoryManager::createImage(vk::Extent3D extent, uint32_t arrayLayers, vk::Format format,
		vk::ImageType type, vk::ImageLayout initialLayout, uint32_t mipLevels, const std::vector<uint32_t>& queueFamilyIndices,
		vk::SampleCountFlagBits sampleCount, vk::SharingMode sharingMode, vk::ImageTiling tiling, vk::ImageUsageFlags usage,
		VmaMemoryUsage memoryUsage, vk::MemoryPropertyFlags requiredMemoryProperties, vk::ImageCreateFlags flags)
	{
		m_memoryMutex.lock();
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
			.flags = flags,
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

		m_memoryMutex.unlock();

		return image;
	}

	void DeviceMemoryManager::destroyBuffer(DeviceBuffer* buffer) {
		m_memoryMutex.lock();
		vmaDestroyBuffer(m_allocator, buffer->buffer, buffer->allocation);
		m_buffers.erase(std::find(m_buffers.begin(), m_buffers.end(), buffer));
		delete buffer;
		buffer = nullptr;
		m_memoryMutex.unlock();

	}

	void DeviceMemoryManager::destroyImage(DeviceImage* texture) {
		m_memoryMutex.lock();
		vmaDestroyImage(m_allocator, texture->image, texture->allocation);
		m_images.erase(std::find(m_images.begin(), m_images.end(), texture));
		delete texture;
		texture = nullptr;
		m_memoryMutex.unlock();
	}

	void DeviceMemoryManager::setCurrentFrameIndex(uint32_t frameIndex) {
		vmaSetCurrentFrameIndex(m_allocator, frameIndex);
	}

	DeviceMemoryStats DeviceMemoryManager::getDeviceMemoryStats() const {
		auto prop = m_physicalDevice.getMemoryProperties();
		VmaBudget* budgets = new VmaBudget[prop.memoryHeapCount];

		vmaGetHeapBudgets(m_allocator, budgets);

		DeviceMemoryStats deviceStats;
		deviceStats.heaps.resize(prop.memoryHeapCount);
		for (int i = 0; i < prop.memoryHeapCount; i++) {

			VmaBudget& budget = budgets[i];
			HeapMemoryStats stats = {};
			stats.flags = (uint32_t)prop.memoryHeaps[i].flags;
			stats.usage = budget.usage;
			stats.budget = budget.budget;
			stats.allocationBytes = budget.statistics.allocationBytes;
			stats.allocationCount = budget.statistics.allocationCount;
			stats.blockBytes = budget.statistics.blockBytes;
			stats.blockCount = budget.statistics.blockCount;
			deviceStats.heaps[i] = std::move(stats);
		}

		delete[] budgets;
		return std::move(deviceStats);
	}
}