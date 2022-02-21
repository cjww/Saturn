#pragma once

namespace vr {
	struct Texture;
}

namespace sa {
	
	class Texture {
	private:
		vr::Texture* m_pTexture;

	public:
		Texture() = delete;
		Texture(vr::Texture* pTexture);

		operator vr::Texture* () {
			return m_pTexture;
		}

	};
}