#include "pch.h"
#include "RenderWindow.hpp"

namespace sa {

	RenderWindow::RenderWindow(uint32_t width, uint32_t height, const char* title) : Window(width, height, title) {
		m_swapchain = Renderer::get().createSwapchain(getWindowHandle());
	}

	RenderWindow::RenderWindow(uint32_t monitorIndex) : Window(monitorIndex) {
		m_swapchain = Renderer::get().createSwapchain(getWindowHandle());
	}


	void RenderWindow::waitForFrame() {
		Renderer::get().waitForFrame(m_swapchain);
	}

	RenderContext RenderWindow::beginFrame() {
		while (isIconified()) {
			waitEvents();
		}
		
		if (wasResized()) {
			m_swapchain = Renderer::get().recreateSwapchain(getWindowHandle(), m_swapchain);
			if (m_resizeCallback) {
				m_resizeCallback(getCurrentExtent());
			}
			setWasResized(false);
			return {};
		}

		return Renderer::get().beginFrame(m_swapchain);
	}

	void RenderWindow::display() {
		Renderer::get().endFrame(m_swapchain);	
	}

	ResourceID RenderWindow::getSwapchainID() const {
		return m_swapchain;
	}

	uint32_t RenderWindow::getSwapchainImageCount() const {
		return Renderer::get().getSwapchainImageCount(m_swapchain);
	}

	void RenderWindow::setResizeCallback(RenderWindowResizeCallback func) {
		m_resizeCallback = func;
	}

}
