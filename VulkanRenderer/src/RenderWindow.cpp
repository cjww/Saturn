#include "pch.h"
#include "RenderWindow.hpp"

namespace sa {

	RenderWindow::RenderWindow(uint32_t width, uint32_t height, const char* title) : Window(width, height, title) {
		m_swapchain = Renderer::Get().createSwapchain(getWindowHandle());
	}

	RenderWindow::RenderWindow(uint32_t monitorIndex) : Window(monitorIndex) {
		m_swapchain = Renderer::Get().createSwapchain(getWindowHandle());
	}


	void RenderWindow::waitForFrame() {
		Renderer::Get().waitForFrame(m_swapchain);
	}

	RenderContext RenderWindow::beginFrame() {
		while (isIconified()) {
			waitEvents();
		}
		
		if (wasResized()) {
			m_swapchain = Renderer::Get().recreateSwapchain(getWindowHandle(), m_swapchain);
			if (m_resizeCallback) {
				m_resizeCallback(getCurrentExtent());
			}
			setWasResized(false);
			return {};
		}

		return Renderer::Get().beginFrame(m_swapchain);
	}

	void RenderWindow::display() {
		Renderer::Get().endFrame(m_swapchain);	
	}

	ResourceID RenderWindow::getSwapchainID() const {
		return m_swapchain;
	}

	uint32_t RenderWindow::getSwapchainImageCount() const {
		return Renderer::Get().getSwapchainImageCount(m_swapchain);
	}

	void RenderWindow::setResizeCallback(RenderWindowResizeCallback func) {
		m_resizeCallback = func;
	}

}
