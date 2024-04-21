#pragma once
#include "Texture.hpp"

#include "Renderer.hpp"

namespace sa {

	class DynamicTexture {
	private:
		std::vector<Texture> m_textures;
		uint32_t m_currentTextureIndex;
		VulkanCore* m_pCore;

		DynamicTexture(const std::vector<Texture>& textures, uint32_t currentIndex);
		

	public:
		DynamicTexture();
		DynamicTexture(const DynamicTexture& other) = default;
		DynamicTexture& operator=(const DynamicTexture& other) = default;

		void create2D(TextureUsageFlags usageFlags, Extent extent, Format format = Format::UNDEFINED, uint32_t mipLevels = 1, uint32_t arrayLayers = 1, uint32_t samples = 1);
		void create2D(const Image& image, bool generateMipmaps);

		void createCube(TextureUsageFlags usageFlags, Extent extent, Format format = Format::UNDEFINED, uint32_t mipLevels = 1, uint32_t samples = 1);
		void createCube(const Image& image, bool generateMipmaps);
		void createCube(const std::vector<Image>& images, bool generateMipmaps);

		void create3D(TextureUsageFlags usageFlags, Extent3D extent, Format format = Format::UNDEFINED, uint32_t mipLevels = 1, uint32_t arrayLayers = 1, uint32_t samples = 1);


		Extent getExtent() const;
		virtual uint32_t getDepth() const;
		vk::ImageView* getView() const;
		TextureUsageFlags getUsageFlags() const;
		TextureType getTextureType() const;

		uint32_t getArrayLayerCount() const;
		uint32_t getMipLevelCount() const;

		const Texture& getTexture(uint32_t index = -1) const;

		uint32_t getTextureIndex() const;
		uint32_t getPreviousTextureIndex() const;
		uint32_t getNextTextureIndex() const;


		uint32_t getTextureCount() const;

		operator const Texture&() const;
		
		bool isValid() const;
		bool isValidImage() const;
		bool isValidView() const;

		void destroy();

		void sync(const RenderContext& context);
		
		bool operator==(const DynamicTexture& other);
		bool operator!=(const DynamicTexture& other);

		std::vector<DynamicTexture> createMipLevelTextures();
		void createMipLevelTextures(uint32_t* count, DynamicTexture* pTextures);

		std::vector<DynamicTexture> createArrayLayerTextures();
		void createArrayLayerTextures(uint32_t* count, DynamicTexture* pTextures);



	};

}