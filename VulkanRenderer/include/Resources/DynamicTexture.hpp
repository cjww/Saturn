#pragma once
#include "Texture.hpp"

#include "Renderer.hpp"

namespace sa {

	class DynamicTexture {
	protected:
		union TextureContainer {
			Texture texture;
			Texture2D texture2D;
			Texture3D texture3D;
			TextureCube textureCube;
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

		const Texture& getTexture() const;

		operator const Texture() const;
		operator Texture() const;

		bool isValid() const;
		bool isValidImage() const;
		bool isValidView() const;

		void destroy();

		void swap();

		bool operator==(const DynamicTexture& other);
		bool operator!=(const DynamicTexture& other);


	};

	class DynamicTexture2D : public DynamicTexture {
	private:

		DynamicTexture2D(const std::vector<Texture2D>& textures);

	public:
		DynamicTexture2D();

		DynamicTexture2D(TextureTypeFlags type, Extent extent, uint32_t sampleCount = 1, uint32_t mipLevels = 1);
		DynamicTexture2D(TextureTypeFlags type, Extent extent,
			FormatPrecisionFlags precisions, FormatDimensionFlags dimensions, FormatTypeFlags types, uint32_t sampleCount = 1, uint32_t mipLevels = 1);


		std::vector<DynamicTexture2D> createMipLevelTextures();
	};


}