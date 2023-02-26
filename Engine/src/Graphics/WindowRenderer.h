#pragma once

#include <RenderWindow.hpp>

namespace sa {
	class WindowRenderer {
	private:
		RenderWindow* m_pWindow;

		ResourceID m_swapchainFramebuffer;
		ResourceID m_swapchainPipeline;
		ResourceID m_swapchainRenderProgram;
		ResourceID m_swapchainDescriptorSet;
		ResourceID m_sampler;

	public:
		WindowRenderer();
		void create(RenderWindow* pTargetWindow);

		void onWindowResize(Extent newExtent);
		void render(RenderContext& context, const Texture& texture);

	};

}