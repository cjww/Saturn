#include "pch.h"
#include "Graphics/IRenderLayer.h"

namespace sa {
	BasicRenderLayer::BasicRenderLayer()
		: m_renderer(Renderer::Get())
		, m_isActive(true)
		, m_isInitialized(false)
	{
	}

	bool BasicRenderLayer::isActive() const {
		return m_isActive;
	}

	void BasicRenderLayer::setActive(bool active) {
		m_isActive = active;
	}

}
