#pragma once
#include "structs.hpp"

namespace sa {
	class Image {
	private:
		unsigned char* m_pixels;
		int m_width, m_height, m_channels;
		
	public:
		Image(const char* path);
		Image(const std::string& path);
		Image(Extent extent, Color color);
		Image(int width, int height, Color color);
		
		Image(Extent extent, unsigned char* pixels, int channels);
		Image(int width, int height, unsigned char* pixels, int channels);


		Image(const Image& other);
		Image& operator=(const Image& other);

		virtual ~Image();

		Extent getExtent() const;
		int getWidth() const;
		int getHeight() const;
		int getChannelCount() const;

		uint32_t calculateMipLevelCount() const;

		unsigned char* getPixels() const;

	};
}