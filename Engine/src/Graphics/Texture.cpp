#include "pch.h"
#include "Texture.h"

namespace sa {

	Texture::Texture(vr::Texture* pTexture) {
		m_pTexture = pTexture;
	}

	Texture::operator vr::Texture* () {
		return m_pTexture;
	}

	sa::Vector2u Texture::getExtent() const {
		return sa::Vector2u(m_pTexture->extent.width, m_pTexture->extent.height);
	}

	bool Texture::isValid() const {
		return m_pTexture != nullptr;
	}


}