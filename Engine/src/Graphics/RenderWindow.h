#pragma once
#include "Window.hpp"


namespace sa {
	class RenderWindow : public Window {
	private:
		uint32_t m_swapchain;
		
	protected:

	public:
		RenderWindow(uint32_t width, uint32_t height, const char* title);
		RenderWindow(uint32_t monitorIndex);


		void frame();

		void display(bool present);

		uint32_t getSwapchainID() const;

	};
}

