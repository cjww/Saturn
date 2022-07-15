#include "pch.h"
#include "Image.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace sa {
	Image::Image(const char* path) {
		m_pixels = stbi_load(path, &m_width, &m_height, &m_channels, STBI_rgb_alpha);
		if (!m_pixels) {
			throw std::runtime_error("Failed to load image " + std::string(path));
		}
		m_channels = 4;
	}
	
	Image::Image(const std::string& path) : Image(path.c_str()){

	}

	Image::Image(Extent extent, Color color) : Image(extent.width, extent.height, color){

	}

	Image::Image(int width, int height, Color color) {
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

	Image::Image(Extent extent, unsigned char* pixels, int channels) : Image(extent.width, extent.height, pixels, channels) {
		
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

	Extent Image::getExtent() const {
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

	uint32_t Image::calculateMipLevelCount() const {
		return static_cast<uint32_t>(std::floor(std::log2(std::max(m_width, m_height)))) + 1;
	}

	unsigned char* Image::getPixels() const {
		return m_pixels;
	}

	uint32_t Image::getPixelSegment(sa::Extent extent, sa::Offset offset, unsigned char* buffer) const {
		uint32_t size = extent.width * extent.height * getChannelCount();
		if (buffer == nullptr) {
			return size;
		}

		uint32_t channelCount = getChannelCount();
		// offset into original image array
		uint32_t byteOffset = channelCount * offset.x + channelCount * offset.y * getWidth();
		// index to last point written to in buffer
		uint32_t bytesWritten = 0;
		// width of segement in bytes
		uint32_t byteWidth = extent.width * channelCount;
		// width off origianl image in bytes
		uint32_t fullByteWidth = getWidth() * channelCount;

		for (uint32_t y = 0; y < extent.height; y++) {
			memcpy(buffer + bytesWritten, m_pixels + byteOffset, byteWidth);
			bytesWritten += byteWidth;
			byteOffset += fullByteWidth;
		}

		return bytesWritten;
	}
}
