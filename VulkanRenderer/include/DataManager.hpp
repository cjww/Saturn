#pragma once

#include "vk_mem_alloc.h"


namespace NAME_SPACE {
	class Resource {
	protected:

	public:
		VmaAllocation allocation;
		Resource() {}
	};

	struct Buffer : public Resource {
		VkBuffer buffer;
		void* mappedData;
		VkDeviceSize size;

		using Resource::Resource;
		Buffer(const Buffer&) = delete;

	};

	struct Texture : public Resource {
		VkImage image;
		VkImageView view;
		VkFormat format;
		VkImageLayout layout;
		VkSampleCountFlagBits sampleCount;
		VkExtent3D extent;

		using Resource::Resource;
		Texture(const Texture&) = delete;

	};

	class DeviceMemoryManager {
	private:
		VmaAllocator m_allocator;
		VmaAllocatorCreateInfo m_allocatorInfo;

		std::vector<uint32_t> m_graphicsQueueFamilyIndices;
		std::vector<uint32_t> m_computeQueueFamilyIndices;

		std::vector<Buffer*> m_buffers;
		std::vector<Texture*> m_textures;
		

		VkFormat getSupportedDepthFormat();
		VkFormat getFormat(const std::vector<VkFormat>& candidates, VkFormatFeatureFlagBits features, VkImageTiling tilling);

	public:
		DeviceMemoryManager(VkInstance instance, VkDevice device, VkPhysicalDevice physicalDevice, uint32_t apiVersion, const std::vector<uint32_t>& graphicsQueueFamilyIndices, const std::vector<uint32_t>& computeQueueFamilyIndices);
		virtual ~DeviceMemoryManager();

		Buffer* createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, void* initialData = nullptr);
		Texture* createImage(VkExtent3D extent, uint32_t arrayLayers, VkFormat format,
			VkImageType type, VkImageLayout initialLayout, uint32_t mipLevels, const std::vector<uint32_t>& queueFamilyIndices,
			VkSampleCountFlagBits sampleCount, VkSharingMode sharingMode, VkImageTiling tiling, VkImageUsageFlags usage,
			VmaMemoryUsage memoryUsage, VkMemoryPropertyFlags requiredMemoryProperties);

		void createImageView(VkImageViewType type, Texture* image,
			VkImageAspectFlags aspectMask, uint32_t baseMiplevel, uint32_t baseArrayLevel);
		void createImageView(VkImageView& view, VkImageViewType type, VkImage image, VkFormat format,
			VkImageAspectFlags aspectMask, uint32_t baseMiplevel, uint32_t baseArrayLevel);

		Texture* createDepthAttachmentTexture2D(VkExtent2D extent, VkSampleCountFlagBits sampleCount, uint32_t mipLevels = 1, uint32_t arrayLayers = 1);
		Texture* createColorTexture2D(VkExtent2D extent, VkImageUsageFlags usage, VkSampleCountFlagBits sampleCount, uint32_t mipLevels = 1, uint32_t arrayLayers = 1);
		Texture* createTexture3D(VkExtent3D extent, VkImageUsageFlags usage, VkSampleCountFlagBits sampleCount, VkFormat format, uint32_t mipLevels = 1, uint32_t arrayLayers = 1);

		void destroyBuffer(Buffer* buffer);
		void destroyImage(Texture* texture);

	};
}