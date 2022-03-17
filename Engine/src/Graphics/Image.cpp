#include "pch.h"
#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace sa {
	Image::Image(const char* path) {
		m_pixels = stbi_load(path, &m_width, &m_height, &m_channels, 0);
		if (!m_pixels) {
			throw std::runtime_error("Failed to load image " + std::string(path));
		}
	}
	
	Image::Image(const std::string& path) : Image(path.c_str()){

	}

	Image::Image(Vector2u extent, Vector4 color) : Image(extent.x, extent.y, color){

	}

	Image::Image(Vector2u extent, Vector3 color) : Image(extent.x, extent.y, color){
		
	}

	Image::Image(int width, int height, Vector4 color) {
		m_width = width;
		m_height = height;
		m_channels = 4;
		int imageSize = m_width * m_height * m_channels;
		m_pixels = new unsigned char[imageSize];
		for (int i = 0; i < imageSize; i += m_channels) {
			m_pixels[i] = (unsigned char)color.r * 255;
			m_pixels[i + 1] = (unsigned char)color.g * 255;
			m_pixels[i + 2] = (unsigned char)color.b * 255;
			m_pixels[i + 3] = (unsigned char)color.a * 255;
		}
	}

	Image::Image(int width, int height, Vector3 color) {
		m_width = width;
		m_height = height;
		m_channels = 3;
		int imageSize = m_width * m_height * m_channels;
		m_pixels = (unsigned char*)STBI_MALLOC(imageSize);
		for (int i = 0; i < imageSize; i += m_channels) {
			m_pixels[i] = (unsigned char)color.r * 255;
			m_pixels[i + 1] = (unsigned char)color.g * 255;
			m_pixels[i + 2] = (unsigned char)color.b * 255;
		}
	}

	Image::Image(Vector2u extent, unsigned char* pixels, int channels) : Image(extent.x, extent.y, pixels, channels) {
		
	}
	
	Image::Image(int width, int height, unsigned char* pixels, int channels) {
		m_width = width;
		m_height = height;
		m_channels = channels;
		m_pixels = pixels;
	}

	Image::Image(const Image& other) {
		m_channels = other.m_channels;
		m_height = other.m_height;
		m_width = other.m_width;
		int imageSize = m_width * m_height * m_channels;
		m_pixels = (unsigned char*)STBI_MALLOC(imageSize);
		memcpy(m_pixels, other.m_pixels, imageSize);
	}

	Image& Image::operator=(const Image& other) {
		if (m_pixels) stbi_image_free(m_pixels);
		m_channels = other.m_channels;
		m_height = other.m_height;
		m_width = other.m_width;
		int imageSize = m_width * m_height * m_channels;
		m_pixels = (unsigned char*)STBI_MALLOC(imageSize);
		memcpy(m_pixels, other.m_pixels, imageSize);
		return *this;
	}

	Image::~Image() {
		stbi_image_free(m_pixels);
	}

	Vector2u Image::getExtent() const {
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
