#include "pch.h"
#include "Graphics/IRenderLayer.h"

namespace sa {
	IRenderLayer::IRenderLayer()
		: m_renderer(Renderer::get())
		, m_isActive(true)
	{
	}


	bool IRenderLayer::isActive() const {
		return m_isActive;
	}

	void IRenderLayer::setActive(bool active) {
		m_isActive = active;
	}

}
