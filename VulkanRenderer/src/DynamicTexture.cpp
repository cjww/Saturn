#include "pch.h"
#include "Resources/DynamicTexture.hpp"
#include "internal/VulkanCore.hpp"

namespace sa {
	DynamicTexture::DynamicTexture() 
		: m_currentTextureIndex(0)
		, m_pCore(Renderer::get().m_pCore.get())
	{

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

	TextureTypeFlags DynamicTexture::getTypeFlags() const {
		return getTexture().getTypeFlags();
	}
	
	const Texture& DynamicTexture::getTexture(uint32_t index) const {
		return m_textures[(index == -1) ? m_currentTextureIndex : index].texture;
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
			tex.texture.destroy();
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
			if (m_textures[i].texture != other.m_textures[i].texture) {
				return false;
			}
		}
		return true;
	}
	
	bool DynamicTexture::operator!=(const DynamicTexture& other) {
		return !(*this == other);
	}
	
	DynamicTexture2D::DynamicTexture2D(const std::vector<Texture2D>& textures, uint32_t currentIndex) {
		m_textures.resize(textures.size());
		for (size_t i = 0; i < textures.size(); i++) {
			m_textures[i].texture2D = textures[i];
			m_textures[i].activeBit = 2;
		}
		m_currentTextureIndex = currentIndex;
	}


	DynamicTexture2D::DynamicTexture2D()
		: DynamicTexture()
	{
	}

	DynamicTexture2D::DynamicTexture2D(TextureTypeFlags type, Extent extent, uint32_t sampleCount, uint32_t mipLevels) : DynamicTexture() {
		m_textures.resize(m_pCore->getQueueCount());
		for (int i = 0; i < m_pCore->getQueueCount(); i++) {
			m_textures[i].texture2D = Texture2D(type, extent, sampleCount, mipLevels);
			m_textures[i].activeBit = 2;

		}
	}

	DynamicTexture2D::DynamicTexture2D(TextureTypeFlags type, Extent extent, FormatPrecisionFlags precisions, FormatDimensionFlags dimensions, FormatTypeFlags types, uint32_t sampleCount, uint32_t mipLevels) : DynamicTexture() {
		m_textures.resize(m_pCore->getQueueCount());
		for (int i = 0; i < m_pCore->getQueueCount(); i++) {
			m_textures[i].texture2D = Texture2D(type, extent, precisions, dimensions, types, sampleCount, mipLevels);
			m_textures[i].activeBit = 2;
		}
	}

	DynamicTexture2D::DynamicTexture2D(TextureTypeFlags type, Extent extent, Format format, uint32_t sampleCount, uint32_t mipLevels) {
		m_textures.resize(m_pCore->getQueueCount());
		for (int i = 0; i < m_pCore->getQueueCount(); i++) {
			m_textures[i].texture2D = Texture2D(type, extent, format, sampleCount, mipLevels);
			m_textures[i].activeBit = 2;
		}
	}

	DynamicTexture2D::operator const Texture2D() const {
		return m_textures[m_currentTextureIndex].texture2D;
	}

	std::vector<DynamicTexture2D> DynamicTexture2D::createMipLevelTextures() {	
		std::vector<DynamicTexture2D> dynamicTextures;

		std::vector<std::vector<Texture2D>> textures;
		for (auto& tex : m_textures) {
			std::vector<Texture2D> mipTextures = tex.texture2D.createMipLevelTextures();
			for (int i = 0; i < mipTextures.size(); i++) {
				if (textures.empty()) 
					textures.resize(mipTextures.size());
				textures[i].push_back(mipTextures[i]);
			}
		}

		for (auto& texArray : textures) {
			dynamicTextures.push_back(DynamicTexture2D(texArray, m_currentTextureIndex));
		}

		return dynamicTextures;
	}

}