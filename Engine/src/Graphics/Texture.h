#pragma once
#include "Tools\Vector.h"
namespace vr {
	struct Texture;
}

namespace sa {
	
	class Texture {
	private:
		vr::Texture* m_pTexture = nullptr;

	public:
		Texture() = default;
		Texture(vr::Texture* pTexture);

		sa::Vector2u getExtent() const;

		bool isValid() const;

		operator vr::Texture*();
	};
}