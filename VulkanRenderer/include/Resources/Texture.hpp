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

	// Wrapper for image pointer
	class Texture2D {
	private:
		VulkanCore* m_pCore;
		DeviceImage* m_pImage;
		DeviceBuffer* m_pStagingBuffer;
		std::shared_ptr<vk::ImageView> m_pView;

		TextureTypeFlags m_type;
		
	public:
		Texture2D(VulkanCore* pCore, TextureTypeFlags type, Extent extent);
		Texture2D(VulkanCore* pCore, TextureTypeFlags type, Extent extent,
			FormatPrecisionFlags precisions, FormatDimensionFlags dimensions, FormatTypeFlags types);
		Texture2D(VulkanCore* pCore, const Image& image);

		Texture2D(const Texture2D&) = default;
		Texture2D& operator=(const Texture2D&) = default;

		void create(TextureTypeFlags type, Extent extent);
		void create(TextureTypeFlags type, Extent extent,
			FormatPrecisionFlags precisions, FormatDimensionFlags dimensions, FormatTypeFlags types);
		void destroy();

		Extent getExtent() const;
		vk::ImageView* getView() const;

		operator const DeviceImage* () const {
			return m_pImage;
		}

	};

}