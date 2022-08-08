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
		TRANSFER_SRC = 1,
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
		
		Texture(VulkanCore* pCore);
		Texture();
	public:

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
		
		bool isValid() const;

		void destroy();

		bool operator==(const Texture& other);

	};

	// Wrapper for image pointer
	class Texture2D : public Texture {
	private:

		void create(TextureTypeFlags type, Extent extent, uint32_t sampleCount, uint32_t mipLevels);
		void create(TextureTypeFlags type, Extent extent,
			FormatPrecisionFlags precisions, FormatDimensionFlags dimensions, FormatTypeFlags types, uint32_t sampleCount, uint32_t mipLevels);
		void create(TextureTypeFlags type, Extent extent, Swapchain* pSwapchain, uint32_t sampleCount);

		friend class Renderer;
		Texture2D(VulkanCore* pCore, TextureTypeFlags type, Extent extent, uint32_t sampleCount = 1);
		Texture2D(VulkanCore* pCore, TextureTypeFlags type, Extent extent,
			FormatPrecisionFlags precisions, FormatDimensionFlags dimensions, FormatTypeFlags types, uint32_t sampleCount = 1);
		Texture2D(VulkanCore* pCore, TextureTypeFlags type, Extent extent,
			Swapchain* pSwapchain, uint32_t sampleCount = 1);

		Texture2D(VulkanCore* pCore, const Image& image, bool generateMipmaps);
	public:
		Texture2D();

		Texture2D(const Texture2D&) = default;
		Texture2D& operator=(const Texture2D&) = default;
		
		
	};

	class TextureCube : public Texture {
	private:
		void create(TextureTypeFlags type, Extent extent, uint32_t sampleCount, uint32_t mipLevels);
		
		friend class Renderer;
		TextureCube(VulkanCore* pCore, const Image& image, bool generateMipmaps);
		TextureCube(VulkanCore* pCore, const std::vector<Image>& images, bool generateMipmaps);
	public:
		TextureCube();

		TextureCube(const TextureCube&) = default;
		TextureCube& operator=(const TextureCube&) = default;
	};

	class Texture3D : public Texture {
	private:
		void create(TextureTypeFlags type, Extent3D extent, uint32_t sampleCount, uint32_t mipLevels, FormatPrecisionFlags formatPercisions, FormatDimensionFlags formatDimensions, FormatTypeFlags formatTypes);
		
		friend class Renderer;
		Texture3D(VulkanCore* pCore, TextureTypeFlags type, Extent3D extent, uint32_t sampleCount, uint32_t mipLevels, FormatPrecisionFlags formatPercisions, FormatDimensionFlags formatDimensions, FormatTypeFlags formatTypes);

	public:
		Texture3D();

		Texture3D(const Texture3D&) = default;
		Texture3D& operator=(const Texture3D&) = default;

	};
}