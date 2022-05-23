#pragma once

namespace vk {
	class ImageView;
}

namespace sa {
	struct DeviceImage;

	class Texture {
	private:
		DeviceImage* m_image;
		std::shared_ptr<vk::ImageView> m_view;
		
	public:
		Texture();


		Extent getExtent();
		std::shared_ptr<vk::ImageView> getView() const;

	};

}