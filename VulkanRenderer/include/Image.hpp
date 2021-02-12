#pragma once
#include "common.hpp"
#include <vulkan/vulkan.hpp>

namespace NAME_SPACE {
	class Image {
	private:
		unsigned char* m_pixels;
		int m_width, m_height, m_channels;
		
	public:
		Image(const char* path);
		Image(const std::string& path);
		Image(const Image& other);
		virtual ~Image();

		VkExtent2D getExtent() const;
		int getWidth() const;
		int getHeight() const;
		int getChannelCount() const;

		unsigned char* getPixels() const;

	};
}