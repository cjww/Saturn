#include "pch.h"
#include "Texture.h"

namespace sa {

	Texture::Texture(vr::Texture* pTexture) {
		m_pTexture = pTexture;
	}

	Texture::operator vr::Texture* () {
		return m_pTexture;
	}

}