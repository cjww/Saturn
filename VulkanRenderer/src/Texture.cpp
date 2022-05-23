#include "pch.h"
#include "Texture.hpp"

namespace sa {

	std::vector<std::shared_ptr<vk::ImageView>> Texture::getViews() const {
		return m_views;
	}
}