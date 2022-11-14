#include "pch.h"
#include "IRenderLayer.h"

namespace sa {
	IRenderLayer::IRenderLayer()
		: m_renderer(Renderer::get())
	{
	}
}