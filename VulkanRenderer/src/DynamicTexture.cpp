#include "pch.h"
#include "Resources/DynamicTexture.hpp"
#include "internal/VulkanCore.hpp"

namespace sa {

	DynamicTexture::DynamicTexture(const std::vector<Texture>& textures, uint32_t currentIndex) {
		m_textures = textures;
		m_currentTextureIndex = currentIndex;
	}

	DynamicTexture::DynamicTexture() 
		: m_currentTextureIndex(0)
		, m_pCore(Renderer::get().m_pCore.get())
	{

	}

	void DynamicTexture::create2D(TextureUsageFlags usageFlags, Extent extent, Format format, uint32_t mipLevels, uint32_t arrayLayers, uint32_t samples) {
		m_textures.resize(m_pCore->getQueueCount());
		for (uint32_t i = 0; i < m_textures.size(); i++) {
			m_textures[i].create2D(usageFlags, extent, format, mipLevels, arrayLayers, samples);
		}
	}
	
	void DynamicTexture::create2D(const Image& image, bool generateMipmaps) {
		m_textures.resize(m_pCore->getQueueCount());
		for (uint32_t i = 0; i < m_textures.size(); i++) {
			m_textures[i].create2D(image, generateMipmaps);
		}
	}
	
	void DynamicTexture::createCube(TextureUsageFlags usageFlags, Extent extent, Format format, uint32_t mipLevels, uint32_t samples) {
		m_textures.resize(m_pCore->getQueueCount());
		for (uint32_t i = 0; i < m_textures.size(); i++) {
			m_textures[i].createCube(usageFlags, extent, format, mipLevels, samples);
		}
	}
	
	void DynamicTexture::createCube(const Image& image, bool generateMipmaps) {
		m_textures.resize(m_pCore->getQueueCount());
		for (uint32_t i = 0; i < m_textures.size(); i++) {
			m_textures[i].createCube(image, generateMipmaps);
		}
	}
	
	void DynamicTexture::createCube(const std::vector<Image>& images, bool generateMipmaps) {
		m_textures.resize(m_pCore->getQueueCount());
		for (uint32_t i = 0; i < m_textures.size(); i++) {
			m_textures[i].createCube(images, generateMipmaps);
		}
	}
	
	void DynamicTexture::create3D(TextureUsageFlags usageFlags, Extent3D extent, Format format, uint32_t mipLevels, uint32_t arrayLayers, uint32_t samples) {
		m_textures.resize(m_pCore->getQueueCount());
		for (uint32_t i = 0; i < m_textures.size(); i++) {
			m_textures[i].create3D(usageFlags, extent, format, mipLevels, arrayLayers, samples);
		}
	}


	Extent DynamicTexture::getExtent() const {
		return getTexture().getExtent();
	}

	uint32_t DynamicTexture::getDepth() const {
		return getTexture().getDepth();
	}

	vk::ImageView* DynamicTexture::getView() const {
		return getTexture().getView();
	}

	TextureUsageFlags DynamicTexture::getUsageFlags() const {
		return getTexture().getUsageFlags();
	}
	
	const Texture& DynamicTexture::getTexture(uint32_t index) const {
		return m_textures[(index == -1) ? m_currentTextureIndex : index];
	}

	uint32_t DynamicTexture::getTextureIndex() const {
		return m_currentTextureIndex;
	}

	uint32_t DynamicTexture::getPreviousTextureIndex() const {
		return (m_currentTextureIndex + m_textures.size() - 1) % m_textures.size();
	}

	uint32_t DynamicTexture::getNextTextureIndex() const {
		return (m_currentTextureIndex + 1) % (uint32_t)m_textures.size();
	}

	uint32_t DynamicTexture::getTextureCount() const {
		return m_textures.size();
	}

	DynamicTexture::operator const Texture() const {
		return getTexture();
	}

	DynamicTexture::operator Texture() const {
		return getTexture();
	}

	bool DynamicTexture::isValid() const {
		return !m_textures.empty() && getTexture().isValid();
	}

	bool DynamicTexture::isValidImage() const {
		return !m_textures.empty() && getTexture().isValidImage();
	}

	bool DynamicTexture::isValidView() const {
		return !m_textures.empty() && getTexture().isValidView();
	}

	bool DynamicTexture::isSampleReady() const {
		return !m_textures.empty() && getTexture().isSampleReady();
	}

	void DynamicTexture::destroy() {
		for (auto& tex : m_textures) {
			tex.destroy();
		}
		m_currentTextureIndex = 0;
	}
	void DynamicTexture::swap() {
		m_currentTextureIndex = (m_currentTextureIndex + 1) % m_textures.size();
	}

	bool DynamicTexture::operator==(const DynamicTexture& other) {
		if (m_textures.size() != other.m_textures.size()) {
			return false;
		}
		for (size_t i = 0; i < m_textures.size(); i++) {
			if (m_textures[i] != other.m_textures[i]) {
				return false;
			}
		}
		return true;
	}
	
	bool DynamicTexture::operator!=(const DynamicTexture& other) {
		return !(*this == other);
	}

	std::vector<DynamicTexture> DynamicTexture::createMipLevelTextures() {
		std::vector<DynamicTexture> dynamicTextures;

		std::vector<std::vector<Texture>> textures;
		for (auto& tex : m_textures) {
			std::vector<Texture> mipTextures = tex.createMipLevelTextures();
			for (int i = 0; i < mipTextures.size(); i++) {
				if (textures.empty())
					textures.resize(mipTextures.size());
				textures[i].push_back(mipTextures[i]);
			}
		}

		for (auto& texArray : textures) {
			dynamicTextures.push_back(DynamicTexture(texArray, m_currentTextureIndex));
		}

		return dynamicTextures;
	}
}