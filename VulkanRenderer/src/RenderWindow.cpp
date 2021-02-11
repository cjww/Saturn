#include "RenderWindow.hpp"

namespace NAME_SPACE {

	void RenderWindow::onResize(GLFWwindow* window, int width, int height) {
		std::cout << width << ", " << height << std::endl;
		RenderWindow* thisWindow = (RenderWindow*)glfwGetWindowUserPointer(window);
		if (thisWindow->m_isIconified) {
			return;
		}
		thisWindow->m_needsRecreation = true;
	}

	void RenderWindow::onIconify(GLFWwindow* window, int iconified) {
		std::cout << "iconify: " << iconified << std::endl;
		RenderWindow* thisWindow = (RenderWindow*)glfwGetWindowUserPointer(window);
		thisWindow->m_isIconified = iconified;
	}

	void RenderWindow::onKey(GLFWwindow* window, int key, int scancode, int action, int mods) {
		RenderWindow* thisWindow = (RenderWindow*)glfwGetWindowUserPointer(window);
#ifdef _WIN32
		if (key == GLFW_KEY_F11 && action == GLFW_PRESS) {
			thisWindow->toggleFullscreen();
		}
#else
		if (scancode == glfwGetKeyScancode(GLFW_KEY_F11) && action == GLFW_PRESS) {
			thisWindow->toggleFullscreen();
		}
#endif // _WIN32

		if (thisWindow->m_onKeyFunction != nullptr) {
			thisWindow->m_onKeyFunction(key, action, mods, scancode);
		}
	}

	void RenderWindow::onMouseButton(GLFWwindow* window, int button, int action, int mods) {
		RenderWindow* w = (RenderWindow*)glfwGetWindowUserPointer(window);
		if (w->m_onMouseButtonFunction != nullptr) {
			w->m_onMouseButtonFunction(button, action, mods);
		}
	}

	void RenderWindow::onClose(GLFWwindow* window) {
		RenderWindow* w = (RenderWindow*)glfwGetWindowUserPointer(window);
		w->close();
	}


	void RenderWindow::create(uint32_t width, uint32_t height, const char* title, GLFWmonitor* monitor) {

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		m_monitor = monitor;
		m_window = glfwCreateWindow(width, height, title, m_monitor, nullptr);
		if (!m_window) {
			throw std::runtime_error("Failed to create GLFW window");
		}

		glfwSetWindowSizeCallback(m_window, &RenderWindow::onResize);
		glfwSetWindowIconifyCallback(m_window, &RenderWindow::onIconify);
		glfwSetKeyCallback(m_window, &RenderWindow::onKey);
		glfwSetMouseButtonCallback(m_window, &RenderWindow::onMouseButton);
		glfwSetWindowCloseCallback(m_window, &RenderWindow::onClose);

		glfwSetWindowUserPointer(m_window, this);

		createSurface(m_window);

		createSwapChain();

		createCommandBuffers();

		createSyncronisationObjects();

		VkExtent2D extent = { width, height };
		m_windowedExtent = extent;

		m_isIconified = false;
		m_needsRecreation = false;

		m_onKeyFunction = nullptr;
		m_onMouseButtonFunction = nullptr;
	}

	void RenderWindow::recreate(VkExtent2D newExtent) {

	}


	void RenderWindow::shutDown() {
		glfwDestroyWindow(m_window);
		m_window = nullptr;


	}

	uint32_t RenderWindow::getNextFrameImage() {
		if (m_needsRecreation) {
			recreate(getCurrentExtent());
			m_needsRecreation = false;
		}
		if (m_isIconified) {
			return -1;
		}
		uint32_t imageIndex = getNextSwapchainImage();
		if (imageIndex == -1) {
			m_needsRecreation = true;
			return -1;
		}
		return imageIndex;
	}

	RenderWindow::RenderWindow(uint32_t width, uint32_t height, const char* title) : Renderer() {
		create(width, height, title, nullptr);
	}

	RenderWindow::RenderWindow(uint32_t monitorIndex) : Renderer() {
		int count;
		GLFWmonitor** monitors = glfwGetMonitors(&count);
		if (monitorIndex >= count || monitorIndex < 0) {
			throw std::runtime_error("Monitor index out of range!");
			return;
		}

		GLFWmonitor* monitor = monitors[monitorIndex];
		int xpos, ypos, width, height;
		glfwGetMonitorWorkarea(monitor, &xpos, &ypos, &width, &height);
		create(width, height, "", monitor);
		m_windowedExtent = { 1000, 600 };
	}

	RenderWindow::~RenderWindow() {
		if (m_window != nullptr) {
			shutDown();
		}
	}

	bool RenderWindow::isOpen() {
		if (m_window == nullptr) {
			return false;
		}
		return !glfwWindowShouldClose(m_window);
	}

	void RenderWindow::pollEvents() {
		glfwPollEvents();
	}

	void RenderWindow::beginFrame() {
		getNextFrameImage();

		vbl::beginPrimaryCommandBuffer(m_graphicsCommandBuffers[m_frameIndex], VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	
		std::vector<VkCommandBuffer> transferBuffers;
		transferBuffers.reserve(m_transferCommandQueue.size());
		for (const auto& t : m_transferCommandQueue) {
			transferBuffers.push_back(t.commandBuffer);
		}
		vkCmdExecuteCommands(m_graphicsCommandBuffers[m_frameIndex], transferBuffers.size(), transferBuffers.data());
	}
	
	void RenderWindow::endFrame() {

		vkEndCommandBuffer(m_graphicsCommandBuffers[m_frameIndex]);

		VkSubmitInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		info.commandBufferCount = 1;
		info.pCommandBuffers = &m_graphicsCommandBuffers[m_frameIndex];
		info.signalSemaphoreCount = 1;
		info.pSignalSemaphores = &m_renderFinishedSemaphore[m_frameIndex];
		info.waitSemaphoreCount = 1;
		info.pWaitSemaphores = &m_imageAvailableSemaphore[m_frameIndex];
		
		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		info.pWaitDstStageMask = &waitStage;

		vkResetFences(m_device, 1, &m_inFlightFences[m_frameIndex]);

		vkQueueSubmit(m_graphicsQueue, 1, &info, m_inFlightFences[m_frameIndex]);
		vbl::presentImage(m_graphicsQueue, m_swapChain.currentImageIndex, m_swapChain.swapChain, m_renderFinishedSemaphore[m_frameIndex]);
		m_frameIndex = (m_frameIndex + 1) % m_swapChain.images.size();

		m_transferCommandQueue.clear();
	}

	void RenderWindow::close() {
		glfwSetWindowShouldClose(m_window, GLFW_TRUE);
	}

	void RenderWindow::setRefreshRate(int rate) {
		int xpos = 0, ypos = 0, width = getCurrentExtent().width, height = getCurrentExtent().height;
		glfwGetWindowPos(m_window, &xpos, &ypos);
		if (m_monitor != nullptr) {
			glfwGetMonitorWorkarea(m_monitor, &xpos, &ypos, &width, &height);
		}
		glfwSetWindowMonitor(m_window, m_monitor, xpos, ypos, width, height, rate);
	}


	void RenderWindow::setMonitor(int monitorIndex) {
		int count;
		GLFWmonitor** monitors = glfwGetMonitors(&count);
		if (monitorIndex >= count) {
			throw std::runtime_error("Monitor index out of range!");
			return;
		}

		int xpos = 100, ypos = 100, width, height;
		if (monitorIndex == -1) {
			m_monitor = nullptr;
			width = m_windowedExtent.width;
			height = m_windowedExtent.height;

		}
		else {
			m_monitor = monitors[monitorIndex];
			glfwGetMonitorWorkarea(m_monitor, &xpos, &ypos, &width, &height);
		}

		glfwSetWindowMonitor(m_window, m_monitor, xpos, ypos, width, height, GLFW_DONT_CARE);
		m_needsRecreation = true;
	}

	void RenderWindow::toggleFullscreen() {
		if (m_monitor == nullptr) {
			setMonitor(0);
		}
		else {
			setMonitor(-1);
		}
	}

	void RenderWindow::setWindowTitle(const char* title) {
		glfwSetWindowTitle(m_window, title);
	}

	void RenderWindow::setWindowTitle(const std::string& title) {
		glfwSetWindowTitle(m_window, title.c_str());
	}

	void RenderWindow::setWindowTitle(std::string&& title) {
		glfwSetWindowTitle(m_window, title.c_str());
	}

	VkExtent2D RenderWindow::getCurrentExtent() const {
		VkExtent2D extent;
		glfwGetWindowSize(m_window, (int*)&extent.width, (int*)&extent.height);
		return extent;
	}

	GLFWwindow* RenderWindow::getWindowHandle() const {
		return m_window;
	}

	int RenderWindow::getKey(int keyCode) const {
		return glfwGetKey(m_window, keyCode);
	}

	int RenderWindow::getMouseButton(int button) const {
		return glfwGetMouseButton(m_window, button);
	}

	glm::vec2 RenderWindow::getCursorPosition() const {
		glm::vec<2, double> pos(0.0);
		glfwGetCursorPos(m_window, &pos.x, &pos.y);
		return static_cast<glm::vec2>(pos);
	}

	void RenderWindow::setCursorPosition(const glm::vec2& position) {
		glfwSetCursorPos(m_window, position.x, position.y);
	}

	void RenderWindow::setHideCursor(bool value) {
		glfwSetInputMode(m_window, GLFW_CURSOR, (value) ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);
	}

	bool RenderWindow::isHidingCursor() const {
		return glfwGetInputMode(m_window, GLFW_CURSOR) == GLFW_CURSOR_HIDDEN;
	}

	void RenderWindow::setKeyCallback(std::function<void(int, int, int, int)> func) {
		m_onKeyFunction = func;
	}

	void RenderWindow::setMouseButtonCallback(std::function<void(int, int, int)> func) {
		m_onMouseButtonFunction = func;
	}
}