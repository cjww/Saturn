#pragma once

#include "debugFunctions.h"

#include "vk_mem_alloc.h"


namespace sa {
	class DeviceResource {
	protected:

	public:
		VmaAllocation allocation;
		DeviceResource() {}
	};

	struct DeviceBuffer : public DeviceResource {
		vk::Buffer buffer;
		void* mappedData;
		vk::DeviceSize size;

		using DeviceResource::DeviceResource;
		DeviceBuffer(const DeviceBuffer&) = delete;

	};

	struct DeviceImage : public DeviceResource {
		vk::Image image;
		vk::Format format;
		vk::ImageLayout layout;
		vk::SampleCountFlagBits sampleCount;
		vk::Extent3D extent;

		uint32_t arrayLayers;
		vk::ImageType imageType;
		uint32_t mipLevels;
		vk::SharingMode sharingMode;
		vk::ImageTiling tiling;
		vk::ImageUsageFlags usage;

		using DeviceResource::DeviceResource;
		DeviceImage(const DeviceImage&) = delete;

	};

	class DeviceMemoryManager {
	private:
		VmaAllocator m_allocator;
		VmaAllocatorCreateInfo m_allocatorInfo;
		
		vk::Instance m_instance;
		vk::PhysicalDevice m_physicalDevice;
		vk::Device m_device;

		std::vector<uint32_t> m_graphicsQueueFamilyIndices;
		std::vector<uint32_t> m_computeQueueFamilyIndices;

		std::vector<DeviceBuffer*> m_buffers;
		std::vector<DeviceImage*> m_images;
		

		vk::Format getSupportedDepthFormat();
		vk::Format getFormat(const std::vector<vk::Format>& candidates, vk::FormatFeatureFlagBits features, vk::ImageTiling tilling);

	public:
		DeviceMemoryManager() = default;
		
		void create(
			vk::Instance instance,
			vk::Device device,
			vk::PhysicalDevice physicalDevice,
			uint32_t apiVersion,
			const std::vector<uint32_t>& graphicsQueueFamilyIndices,
			const std::vector<uint32_t>& computeQueueFamilyIndices
		);
		
		void destroy();

		DeviceBuffer* createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, VmaMemoryUsage memoryUsage, void* initialData = nullptr);
		DeviceImage* createImage(
			vk::Extent3D extent, 
			uint32_t arrayLayers, 
			vk::Format format,
			vk::ImageType type, 
			vk::ImageLayout initialLayout, 
			uint32_t mipLevels, 
			const std::vector<uint32_t>& queueFamilyIndices,
			vk::SampleCountFlagBits sampleCount, 
			vk::SharingMode sharingMode, 
			vk::ImageTiling tiling, 
			vk::ImageUsageFlags usage,
			VmaMemoryUsage memoryUsage, 
			vk::MemoryPropertyFlags requiredMemoryProperties
		);

		DeviceImage* createDepthAttachmentTexture2D(vk::Extent2D extent, vk::SampleCountFlagBits sampleCount, uint32_t mipLevels = 1, uint32_t arrayLayers = 1);
		DeviceImage* createColorTexture2D(vk::Extent2D extent, vk::ImageUsageFlags usage, vk::Format format, vk::SampleCountFlagBits sampleCount, uint32_t mipLevels, uint32_t arrayLayers);
		DeviceImage* createTexture3D(vk::Extent3D extent, vk::ImageUsageFlags usage, vk::SampleCountFlagBits sampleCount, vk::Format format, uint32_t mipLevels = 1, uint32_t arrayLayers = 1);

		void destroyBuffer(DeviceBuffer* buffer);
		void destroyImage(DeviceImage* texture);

	};
}