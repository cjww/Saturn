#include "pch.h"
#include "Graphics/IRenderLayer.h"

namespace sa {
	IRenderLayer::IRenderLayer()
		: m_renderer(Renderer::get())
		, m_isActive(true)
		, m_isInitialized(false)
	{
	}


	bool IRenderLayer::isActive() const {
		return m_isActive;
	}

	void IRenderLayer::setActive(bool active) {
		m_isActive = active;
	}

}
