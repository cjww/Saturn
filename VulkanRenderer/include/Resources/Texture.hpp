#pragma once

#include "structs.hpp"
#include "Image.hpp"
#include "FormatFlags.hpp"
#include "Format.hpp"
#include "ResourceManager.hpp"

namespace vk {
	class ImageView;
}

namespace sa {
	struct DeviceImage;
	struct DeviceBuffer;
	struct DataTransfer;

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

	enum TextureViewType {
		VIEW_TYPE_1D,
		VIEW_TYPE_2D,
		VIEW_TYPE_3D,
		VIEW_TYPE_CUBE,
		VIEW_TYPE_1D_ARRAY,
		VIEW_TYPE_2D_ARRAY,
		VIEW_TYPE_CUBE_ARRAY
	};

	class Texture {
	protected:
		VulkanCore* m_pCore;
		DeviceImage* m_pImage;
		DeviceBuffer* m_pStagingBuffer;
		ResourceID m_view;

		DataTransfer* m_pDataTransfer;

		TextureTypeFlags m_type;
		
		Texture(VulkanCore* pCore);


		ResourceID createImageView(TextureViewType viewType, uint32_t mipLevels, uint32_t baseMipLevel, uint32_t layers, uint32_t baseArrayLevel);
		

	public:
		Texture();

		Extent getExtent() const;
		virtual uint32_t getDepth() const;
		vk::ImageView* getView() const;
		TextureTypeFlags getTypeFlags() const;

		uint32_t getArrayLayerCount() const;
		uint32_t getMipLevelCount() const;
		
		operator const DeviceImage* () const {
			return m_pImage;
		}

		operator DeviceImage* () const {
			return m_pImage;
		}
		
		bool isValid() const;
		bool isValidImage() const;
		bool isValidView() const;

		bool isSampleReady() const;

		void destroy();

		bool operator==(const Texture& other);
		bool operator!=(const Texture& other);


	};

	// Wrapper for image pointer
	class Texture2D : public Texture {
	public:
		
	private:
		
		void create(TextureTypeFlags type, Extent extent, uint32_t sampleCount, uint32_t mipLevels, uint32_t arrayLayers);
		void create(TextureTypeFlags type, Extent extent, Swapchain* pSwapchain, uint32_t sampleCount);
		void create(TextureTypeFlags type, Extent extent, Format format, uint32_t sampleCount, uint32_t mipLevels, uint32_t arrayLayers);

		Texture2D(TextureTypeFlags type, Extent extent,
			Swapchain* pSwapchain, uint32_t sampleCount = 1);
		Texture2D(ResourceID imageView);

	public:
		Texture2D(TextureTypeFlags type, Extent extent, uint32_t sampleCount = 1, uint32_t mipLevels = 1, uint32_t arrayLayers = 1);
		Texture2D(TextureTypeFlags type, Extent extent, Format format, uint32_t sampleCount = 1, uint32_t mipLevels = 1, uint32_t arrayLayers = 1);

		Texture2D(const Image& image, bool generateMipmaps);
		
		Texture2D();
		Texture2D(const Texture2D&) = default;
		Texture2D& operator=(const Texture2D&) = default;
		
		std::vector<Texture2D> createMipLevelTextures();
		void createMipLevelTextures(uint32_t* count, Texture2D* pTextures);

		std::vector<Texture2D> createArrayLayerTextures();
		void createArrayLayerTextures(uint32_t* count, Texture2D* pTextures);


	};

	class TextureCube : public Texture {
	private:
		void create(TextureTypeFlags type, Extent extent, uint32_t sampleCount, uint32_t mipLevels, Format format);
		
	public:
		TextureCube(const Image& image, bool generateMipmaps, Format format);
		TextureCube(const std::vector<Image>& images, bool generateMipmaps, Format format);
		TextureCube();

		TextureCube(const TextureCube&) = default;
		TextureCube& operator=(const TextureCube&) = default;
	};

	class Texture3D : public Texture {
	private:
		void create(TextureTypeFlags type, Extent3D extent, uint32_t sampleCount, uint32_t mipLevels, Format format);
	public:
		Texture3D(TextureTypeFlags type, Extent3D extent, uint32_t sampleCount, uint32_t mipLevels, Format format);
		Texture3D();

		Texture3D(const Texture3D&) = default;
		Texture3D& operator=(const Texture3D&) = default;

	};


}