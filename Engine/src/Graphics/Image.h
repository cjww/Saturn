#pragma once
#include <Tools\Vector.h>

namespace sa {
	class Image {
	private:
		unsigned char* m_pixels;
		int m_width, m_height, m_channels;
		
	public:
		Image(const char* path);
		Image(const std::string& path);
		Image(Vector2u extent, Vector4 color);
		Image(Vector2u extent, Vector3 color);
		Image(int width, int height, Vector4 color);
		Image(int width, int height, Vector3 color);

		Image(Vector2u extent, unsigned char* pixels, int channels);
		Image(int width, int height, unsigned char* pixels, int channels);


		Image(const Image& other);
		Image& operator=(const Image& other);

		virtual ~Image();

		Vector2u getExtent() const;
		int getWidth() const;
		int getHeight() const;
		int getChannelCount() const;

		unsigned char* getPixels() const;

	};
}