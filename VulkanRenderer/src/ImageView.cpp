#include "pch.h"
#include "Resources/ImageView.hpp"

#include "internal\VulkanCore.hpp"
#include "Renderer.hpp"

namespace sa {
	void ImageView::create(DeviceImage* pImage, vk::ImageViewType* pViewType, uint32_t* pAspectFlags, uint32_t mipLevels, uint32_t baseMipLevel, uint32_t arrayLayers, uint32_t baseArrayLayer) {
		VulkanCore* pCore = Renderer::Get().getCore();

		m_imageView = ResourceManager::Get().insert<vk::ImageView>(pCore->createImageView(
			*pViewType,
			pImage->image,
			pImage->format,
			static_cast<vk::ImageAspectFlags>(*pAspectFlags),
			mipLevels,
			baseMipLevel,
			arrayLayers,
			baseArrayLayer
		));
		//ResourceManager::Get().printContainer<vk::ImageView>();

		m_baseArrayLayer = baseArrayLayer;
		m_arrayLayerCount = arrayLayers;
		m_baseMipLevel = baseMipLevel;
		m_mipLevelCount = mipLevels;
		m_pImage = pImage;
	}

	void ImageView::destroy() {
		ResourceManager::Get().remove<vk::ImageView>(m_imageView);
		m_imageView = NULL_RESOURCE;
	}

	bool ImageView::isValid() const {
		return m_imageView != NULL_RESOURCE;
	}

	uint32_t ImageView::getArrayLayerCount() const {
		return m_arrayLayerCount;
	}

	uint32_t ImageView::getBaseArrayLayer() const {
		return m_baseArrayLayer;
	}

	uint32_t ImageView::getMipLevelCount() const {
		return m_mipLevelCount;
	}

	uint32_t ImageView::getBaseMipLevel() const {
		return m_baseMipLevel;
	}

	Format ImageView::getFormat() const {
		return static_cast<Format>(m_pImage->format);
	}

	vk::ImageView* ImageView::getView() const {
		return ResourceManager::Get().get<vk::ImageView>(m_imageView);
	}

	const DeviceImage* ImageView::getImage() const {
		return m_pImage;
	}

	ImageView::operator vk::ImageView* () const {
		return getView();
	}
}
