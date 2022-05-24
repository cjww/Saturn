#pragma once

#include "structs.hpp"


namespace vk {
	class ImageView;
}

namespace sa {
	struct DeviceImage;

	// Wrapper for image pointer
	class Texture2D {
	private:
		DeviceImage* m_pImage;
		std::shared_ptr<vk::ImageView> m_pView;
		
	public:
		Texture2D(DeviceImage* pImage, std::shared_ptr<vk::ImageView> pView);
		Texture2D(const Texture2D&) = default;
		Texture2D& operator=(const Texture2D&) = default;

		Extent getExtent() const;
		vk::ImageView* getView() const;

	};

}