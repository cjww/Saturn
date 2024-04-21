#pragma once

#include "Format.hpp"

namespace vk {
	class ImageView;
	enum class ImageViewType;
}

namespace sa {
	class DeviceImage;

	class ImageView {
	private:
		uint32_t m_baseArrayLayer;
		uint32_t m_arrayLayerCount;

		uint32_t m_baseMipLevel;
		uint32_t m_mipLevelCount;

		ResourceID m_imageView = NULL_RESOURCE;
		DeviceImage* m_pImage;

	public:
		void create(DeviceImage* pImage, vk::ImageViewType* pViewType, uint32_t* pAspectFlags, uint32_t mipLevels, uint32_t baseMipLevel, uint32_t arrayLayers, uint32_t baseArrayLayer);
		void destroy();

		bool isValid() const;

		uint32_t getArrayLayerCount() const;
		uint32_t getBaseArrayLayer() const;
		uint32_t getMipLevelCount() const;
		uint32_t getBaseMipLevel() const;

		Format getFormat() const;

		vk::ImageView* getView() const;
		const DeviceImage* getImage() const;

		operator vk::ImageView*() const;

	};

}