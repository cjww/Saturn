#pragma once

#include "structs.hpp"
#include "Image.hpp"
#include "FormatFlags.hpp"

namespace vk {
	class ImageView;
}

namespace sa {
	struct DeviceImage;
	struct DeviceBuffer;
	class Swapchain;

	class VulkanCore;

	typedef uint32_t TextureTypeFlags;
	enum TextureTypeFlagBits : TextureTypeFlags {
		TRANSFER_DST = 2,
		SAMPLED = 4,
		STORAGE = 8,
		COLOR_ATTACHMENT = 16,
		DEPTH_ATTACHMENT = 32,
		INPUT_ATTACHMENT = 128,
	};

	class Texture {
	protected:
		VulkanCore* m_pCore;
		DeviceImage* m_pImage;
		DeviceBuffer* m_pStagingBuffer;
		ResourceID m_view;

		TextureTypeFlags m_type;
	public:
		Texture(VulkanCore* pCore);
		Extent getExtent() const;
		virtual uint32_t getDepth() const;
		vk::ImageView* getView() const;
		TextureTypeFlags getTypeFlags() const;
		
		operator const DeviceImage* () const {
			return m_pImage;
		}

		operator DeviceImage* () const {
			return m_pImage;
		}

	};

	// Wrapper for image pointer
	class Texture2D : public Texture {
	private:

	public:
		Texture2D(VulkanCore* pCore, TextureTypeFlags type, Extent extent, uint32_t sampleCount = 1);
		Texture2D(VulkanCore* pCore, TextureTypeFlags type, Extent extent,
			FormatPrecisionFlags precisions, FormatDimensionFlags dimensions, FormatTypeFlags types, uint32_t sampleCount = 1);
		Texture2D(VulkanCore* pCore, TextureTypeFlags type, Extent extent,
			Swapchain* pSwapchain, uint32_t sampleCount = 1);

		Texture2D(VulkanCore* pCore, const Image& image);

		Texture2D(const Texture2D&) = default;
		Texture2D& operator=(const Texture2D&) = default;

		void create(TextureTypeFlags type, Extent extent, uint32_t sampleCount);
		void create(TextureTypeFlags type, Extent extent,
			FormatPrecisionFlags precisions, FormatDimensionFlags dimensions, FormatTypeFlags types, uint32_t sampleCount);
		void create(TextureTypeFlags type, Extent extent, Swapchain* pSwapchain, uint32_t sampleCount);


		void destroy();

	};

}