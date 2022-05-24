#include "pch.h"
#include "Texture.hpp"

#include "Resources\DeviceMemoryManager.hpp"

namespace sa {
	Texture2D::Texture2D(DeviceImage* pImage, std::shared_ptr<vk::ImageView> pView) 
		: m_pImage(pImage)
		, m_pView(pView)
	{
	}
	
	Extent Texture2D::getExtent() const {
		return { m_pImage->extent.width, m_pImage->extent.height };
	}

	vk::ImageView* Texture2D::getView() const {
		return m_pView.get();
	}
}