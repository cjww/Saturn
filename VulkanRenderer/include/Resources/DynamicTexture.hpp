#pragma once
#include "Texture.hpp"

#include "Renderer.hpp"

namespace sa {

	class DynamicTexture {
	protected:
		
		struct TextureContainer {
			uint8_t activeBit;
			union {
				Texture texture;		// 1
				Texture2D texture2D;	// 2
				Texture3D texture3D;	// 4
				TextureCube textureCube;// 8
			};
			
			TextureContainer() : activeBit(0) {}
			TextureContainer(const Texture& texture) : texture(texture), activeBit(1) {}
			TextureContainer(const Texture2D& texture) : texture2D(texture), activeBit(2) {}
			TextureContainer(const Texture3D& texture) : texture3D(texture), activeBit(4) {}
			TextureContainer(const TextureCube& texture) : textureCube(texture), activeBit(8) {}

			TextureContainer(const TextureContainer& other) {
				*this = other;
			}

			TextureContainer& operator=(const TextureContainer& other) {
				switch (other.activeBit) {
				case 1:
					texture = other.texture;
					break;
				case 2:
					texture2D = other.texture2D;
					break;
				case 4:
					texture3D = other.texture3D;
					break;
				case 8:
					textureCube = other.textureCube;
					break;
				default:
					break;
				}
				activeBit = other.activeBit;
				return *this;
			}
		};

		std::vector<TextureContainer> m_textures;
		uint32_t m_currentTextureIndex;
		VulkanCore* m_pCore;


	public:
		DynamicTexture();
		DynamicTexture(const DynamicTexture& other) = default;
		DynamicTexture& operator=(const DynamicTexture& other) = default;

		Extent getExtent() const;
		virtual uint32_t getDepth() const;
		vk::ImageView* getView() const;
		TextureTypeFlags getTypeFlags() const;

		const Texture& getTexture(uint32_t index = -1) const;

		uint32_t getTextureIndex() const;
		uint32_t getPreviousTextureIndex() const;
		uint32_t getNextTextureIndex() const;


		uint32_t getTextureCount() const;

		operator const Texture() const;
		operator Texture() const;

		bool isValid() const;
		bool isValidImage() const;
		bool isValidView() const;

		bool isSampleReady() const;

		void destroy();

		void swap();
		
		bool operator==(const DynamicTexture& other);
		bool operator!=(const DynamicTexture& other);


	};

	class DynamicTexture2D : public DynamicTexture {
	private:

		DynamicTexture2D(const std::vector<Texture2D>& textures, uint32_t currentIndex);

	public:
		DynamicTexture2D();

		DynamicTexture2D(TextureTypeFlags type, Extent extent, uint32_t sampleCount = 1, uint32_t mipLevels = 1);
		DynamicTexture2D(TextureTypeFlags type, Extent extent, Format format, uint32_t sampleCount = 1, uint32_t mipLevels = 1);

		operator const Texture2D() const;

		using DynamicTexture::operator Texture;


		std::vector<DynamicTexture2D> createMipLevelTextures();

	};


}