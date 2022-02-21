#pragma once
#include "common.hpp"
#include <glm\vec3.hpp>
#include <glm\vec4.hpp>;

namespace NAME_SPACE {
	class Image {
	private:
		unsigned char* m_pixels;
		int m_width, m_height, m_channels;
		
	public:
		Image(const char* path);
		Image(const std::string& path);
		Image(VkExtent2D extent, glm::vec4 color);
		Image(VkExtent2D extent, glm::vec3 color);
		Image(int width, int height, glm::vec4 color);
		Image(int width, int height, glm::vec3 color);

		Image(VkExtent2D extent, unsigned char* pixels, int channels);
		Image(int width, int height, unsigned char* pixels, int channels);


		Image(const Image& other);
		Image& operator=(const Image& other);

		virtual ~Image();

		VkExtent2D getExtent() const;
		int getWidth() const;
		int getHeight() const;
		int getChannelCount() const;

		unsigned char* getPixels() const;

	};
}