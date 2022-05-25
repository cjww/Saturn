#include "pch.h"
#include "RenderWindow.hpp"

namespace sa {

	RenderWindow::RenderWindow(uint32_t width, uint32_t height, const char* title) : Window(width, height, title) {
		m_swapchain = sa::Renderer::get().createSwapchain(getWindowHandle());
	}

	RenderWindow::RenderWindow(uint32_t monitorIndex) : Window(monitorIndex) {
		m_swapchain = sa::Renderer::get().createSwapchain(getWindowHandle());
	}


	RenderContext RenderWindow::beginFrame() {
		while (isIconified()) { // TODO: better way?
			pollEvents();
		}

		return sa::Renderer::get().beginFrame(m_swapchain);
	}

	void RenderWindow::display() {
		sa::Renderer::get().endFrame(m_swapchain);	
	}

	ResourceID RenderWindow::getSwapchainID() const {
		return m_swapchain;
	}

	uint32_t RenderWindow::getSwapchainImageCount() const {
		return Renderer::get().getSwapchainImageCount(m_swapchain);
	}

}
