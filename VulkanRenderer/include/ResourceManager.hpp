#pragma once

#include "common.hpp"
#include "vk_mem_alloc.h"
#include <memory>
#include <vector>


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

	struct Image : public Resource {
		VkImage image;
		VkImageView view;
		VkFormat format;
		VkImageLayout layout;
		VkSampleCountFlagBits sampleCount;
		VkExtent3D extent;

		using Resource::Resource;
		Image(const Image&) = delete;

	};

	typedef std::shared_ptr<Buffer> BufferPtr;
	typedef std::shared_ptr<Image> ImagePtr;
	

	class ResourceManager {
	private:
		VmaAllocator m_allocator;
		VmaAllocatorCreateInfo m_allocatorInfo;

		std::vector<uint32_t> m_graphicsQueueFamilyIndices;
		std::vector<uint32_t> m_computeQueueFamilyIndices;


		VkFormat getSupportedDepthFormat();
		VkFormat getFormat(const std::vector<VkFormat>& candidates, VkFormatFeatureFlagBits features, VkImageTiling tilling);

	public:
		ResourceManager(VkInstance instance, VkDevice device, VkPhysicalDevice physicalDevice, uint32_t apiVersion, const std::vector<uint32_t>& graphicsQueueFamilyIndices, const std::vector<uint32_t>& computeQueueFamilyIndices);
		virtual ~ResourceManager();

		std::shared_ptr<Buffer> createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, void* initialData = nullptr);
		std::shared_ptr<Image> createImage(VkExtent3D extent, uint32_t arrayLayers, VkFormat format,
			VkImageType type, VkImageLayout initialLayout, uint32_t mipLevels, const std::vector<uint32_t>& queueFamilyIndices,
			VkSampleCountFlagBits sampleCount, VkSharingMode sharingMode, VkImageTiling tiling, VkImageUsageFlags usage,
			VmaMemoryUsage memoryUsage, VkMemoryPropertyFlags requiredMemoryProperties);

		void createImageView(VkImageViewType type, std::shared_ptr<Image> image,
			VkImageAspectFlags aspectMask, uint32_t baseMiplevel, uint32_t baseArrayLevel);
		void createImageView(VkImageView& view, VkImageViewType type, VkImage image, VkFormat format,
			VkImageAspectFlags aspectMask, uint32_t baseMiplevel, uint32_t baseArrayLevel);


		std::shared_ptr<Image> createDepthImage(VkExtent2D extent);
		ImagePtr createShaderReadOnlyColorImage2D(VkExtent2D extent);

	};
}