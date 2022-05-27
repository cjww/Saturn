#pragma once

#include "structs.hpp"
#include "Image.hpp"

namespace vk {
	class ImageView;
}

namespace sa {
	struct DeviceImage;
	struct DeviceBuffer;

	class VulkanCore;

	enum class TextureType {
		SAMPLED_COLOR,
		ATTACHMENT_COLOR,
		ATTACHMENT_DEPTH,
	};

	// Wrapper for image pointer
	class Texture2D {
	private:
		VulkanCore* m_pCore;
		DeviceImage* m_pImage;
		DeviceBuffer* m_pStagingBuffer;
		std::shared_ptr<vk::ImageView> m_pView;

		TextureType m_type;
		
	public:
		Texture2D(VulkanCore* pCore, TextureType type, Extent extent);
		Texture2D(VulkanCore* pCore, const Image& image);

		Texture2D(const Texture2D&) = default;
		Texture2D& operator=(const Texture2D&) = default;

		void create(TextureType type, Extent extent);
		void destroy();

		Extent getExtent() const;
		vk::ImageView* getView() const;

		operator const DeviceImage* () const {
			return m_pImage;
		}

	};

}