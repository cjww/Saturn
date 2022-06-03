#pragma once
#include "Window.hpp"
#include "Renderer.hpp"
namespace sa {
	typedef std::function<void(Extent)> RenderWindowResizeCallback;
	
	class RenderWindow : public Window {
	private:
		ResourceID m_swapchain;
		RenderWindowResizeCallback m_resizeCallback;
	protected:

	public:
		RenderWindow(uint32_t width, uint32_t height, const char* title);
		RenderWindow(uint32_t monitorIndex);


		RenderContext beginFrame();

		void display();

		ResourceID getSwapchainID() const;
		uint32_t getSwapchainImageCount() const;

		void setResizeCallback(RenderWindowResizeCallback func);
	};
}

