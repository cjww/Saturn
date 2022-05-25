#pragma once
#include "Window.hpp"
#include "Renderer.hpp"
namespace sa {
	class RenderWindow : public Window {
	private:
		ResourceID m_swapchain;

	protected:

	public:
		RenderWindow(uint32_t width, uint32_t height, const char* title);
		RenderWindow(uint32_t monitorIndex);


		RenderContext beginFrame();

		void display();

		ResourceID getSwapchainID() const;
		uint32_t getSwapchainImageCount() const;
	};
}

