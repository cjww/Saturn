#include "pch.h"
#include "RenderWindow.h"

#include "Graphics\Vulkan\Renderer.hpp"
#include "GLFW\glfw3.h"

#include <thread>

namespace sa {

	RenderWindow::RenderWindow(uint32_t width, uint32_t height, const char* title) : Window(width, height, title) {
		m_swapchain = vr::Renderer::get().createSwapchain(getWindowHandle());
	}

	RenderWindow::RenderWindow(uint32_t monitorIndex) : Window(monitorIndex) {
		m_swapchain = vr::Renderer::get().createSwapchain(getWindowHandle());
	}


	bool RenderWindow::frame() {
		while (isIconified()) { // TODO: better way?
			pollEvents();
		}

		return vr::Renderer::get().beginFrame(m_swapchain);
	}

	void RenderWindow::display(bool present) {
		vr::Renderer::get().endFrame();
		vr::Renderer::get().submit();
		if(present)
			vr::Renderer::get().present(m_swapchain);
	}

	uint32_t RenderWindow::getSwapchainID() const {
		return m_swapchain;
	}

}