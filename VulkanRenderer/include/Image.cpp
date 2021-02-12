#include "Image.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace NAME_SPACE {
	Image::Image(const char* path) {
		m_pixels = stbi_load(path, &m_width, &m_height, &m_channels, 0);
		if (!m_pixels) {
			throw std::runtime_error("Failed to load image " + std::string(path));
		}
	}
	
	Image::Image(const std::string& path) : Image(path.c_str()){

	}

	Image::Image(const Image& other) {
		m_channels = other.m_channels;
		m_height = other.m_height;
		m_width = other.m_width;
		int imageSize = m_width * m_height * m_channels;
		m_pixels = (unsigned char*)STBI_MALLOC(imageSize);
		memcpy(m_pixels, other.m_pixels, imageSize);
	}

	Image::~Image() {
		stbi_image_free(m_pixels);
	}

	VkExtent2D Image::getExtent() const {
		return { static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height) };
	}

	int Image::getWidth() const {
		return m_width;
	}

	int Image::getHeight() const {
		return m_height;
	}

	int Image::getChannelCount() const {
		return m_channels;
	}

	unsigned char* Image::getPixels() const {
		return m_pixels;
	}
}
