#include "pch.h"
#include "Window.hpp"

#include "GLFW/glfw3.h"

namespace sa {

	void Window::onResize(GLFWwindow* window, int width, int height) {
		Window* thisWindow = (Window*)glfwGetWindowUserPointer(window);
		if (thisWindow->m_isIconified) {
			return;
		}
		thisWindow->m_wasResized = true;
	}

	void Window::onIconify(GLFWwindow* window, int iconified) {
		Window* thisWindow = (Window*)glfwGetWindowUserPointer(window);
		thisWindow->m_isIconified = iconified;
	}

	void Window::onKey(GLFWwindow* window, int key, int scancode, int action, int mods) {
		Window* thisWindow = (Window*)glfwGetWindowUserPointer(window);
	#ifdef _WIN32
		if (key == GLFW_KEY_F11 && action == GLFW_PRESS) {
			thisWindow->toggleFullscreen();
		}
	#endif // _WIN32

		if (thisWindow->m_onKeyFunction != nullptr) {
			thisWindow->m_onKeyFunction((Key)key, (InputAction)action, (ModKeyFlags)mods, scancode);
		}
	}

	void Window::onMouseButton(GLFWwindow* window, int button, int action, int mods) {
		Window* w = (Window*)glfwGetWindowUserPointer(window);
		if (w->m_onMouseButtonFunction != nullptr) {
			w->m_onMouseButtonFunction((MouseButton)button, (InputAction)action, (ModKeyFlags)mods);
		}
	}

	void Window::onClose(GLFWwindow* window) {
		Window* w = (Window*)glfwGetWindowUserPointer(window);
		w->close();
	}


	void Window::create(uint32_t width, uint32_t height, const char* title, GLFWmonitor* monitor) {
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		m_monitor = monitor;
		m_window = glfwCreateWindow(width, height, title, m_monitor, nullptr);
		if (!m_window) {
			throw std::runtime_error("Failed to create GLFW window");
		}

		glfwSetWindowSizeCallback(m_window, &Window::onResize);
		glfwSetWindowIconifyCallback(m_window, &Window::onIconify);
		glfwSetKeyCallback(m_window, &Window::onKey);
		glfwSetMouseButtonCallback(m_window, &Window::onMouseButton);
		glfwSetWindowCloseCallback(m_window, &Window::onClose);

		glfwSetWindowUserPointer(m_window, this);

		Extent extent = { width, height };
		m_windowedExtent = extent;

		m_isIconified = false;
		m_wasResized = false;

		m_onKeyFunction = nullptr;
		m_onMouseButtonFunction = nullptr;
	}

	void Window::shutDown() {
		glfwDestroyWindow(m_window);
		m_window = nullptr;
	}

	Window::Window(uint32_t width, uint32_t height, const char* title) {
		if (s_windowCount == 0) {
			if (!glfwInit()) {
				throw std::runtime_error("Failed to initialize GLFW!");
			}
		}
		s_windowCount++;
		create(width, height, title, nullptr);
	}

	Window::Window(uint32_t monitorIndex) {
		if (s_windowCount == 0) {
			if (!glfwInit()) {
				throw std::runtime_error("Failed to initialize GLFW!");
			}
		}
		s_windowCount++;

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

	Window::~Window() {
		if (m_window != nullptr) {
			shutDown();

			s_windowCount--;
			if (s_windowCount == 0) {
				glfwTerminate();
			}
		}
	}

	bool Window::isOpen() {
		if (m_window == nullptr) {
			return false;
		}
		return !glfwWindowShouldClose(m_window);
	}

	void Window::pollEvents() {
		glfwPollEvents();
	}

	void Window::close() {
		glfwSetWindowShouldClose(m_window, GLFW_TRUE);
	}

	void Window::setRefreshRate(int rate) {
		int xpos = 0, ypos = 0, width = getCurrentExtent().width, height = getCurrentExtent().height;
		glfwGetWindowPos(m_window, &xpos, &ypos);
		if (m_monitor != nullptr) {
			glfwGetMonitorWorkarea(m_monitor, &xpos, &ypos, &width, &height);
		}
		glfwSetWindowMonitor(m_window, m_monitor, xpos, ypos, width, height, rate);
	}


	void Window::setMonitor(int monitorIndex) {
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
		m_wasResized = true;
	}

	void Window::toggleFullscreen() {
		if (m_monitor == nullptr) {
			setMonitor(0);
		}
		else {
			setMonitor(-1);
		}
	}

	void Window::setWindowTitle(const char* title) {
		glfwSetWindowTitle(m_window, title);
	}

	void Window::setWindowTitle(const std::string& title) {
		glfwSetWindowTitle(m_window, title.c_str());
	}

	void Window::setWindowTitle(std::string&& title) {
		glfwSetWindowTitle(m_window, title.c_str());
	}

	Extent Window::getCurrentExtent() const {
		int width, height;
		glfwGetFramebufferSize(m_window, &width, &height);
		return { (uint32_t)width, (uint32_t)height };
	}

	GLFWwindow* Window::getWindowHandle() const {
		return m_window;
	}

	int Window::getKey(Key keyCode) const {
		return glfwGetKey(m_window, (int)keyCode);
	}

	int Window::getMouseButton(MouseButton button) const {
		return glfwGetMouseButton(m_window, (int)button);
	}

	Point Window::getCursorPosition() const {
		Point pos;
		glfwGetCursorPos(m_window, &pos.x, &pos.y);
		return pos;
	}

	void Window::setCursorPosition(const Point& position) {
		glfwSetCursorPos(m_window, position.x, position.y);
	}

	void Window::setHideCursor(bool value) {
		glfwSetInputMode(m_window, GLFW_CURSOR, (value) ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);
	}

	bool Window::isHidingCursor() const {
		return glfwGetInputMode(m_window, GLFW_CURSOR) == GLFW_CURSOR_HIDDEN;
	}

	void Window::setKeyCallback(KeyCallback func) {
		m_onKeyFunction = func;
	}

	void Window::setMouseButtonCallback(MouseButtonCallback func) {
		m_onMouseButtonFunction = func;
	}

	void Window::setWasResized(bool value) {
		m_wasResized = value;
	}

	bool Window::wasResized() const {
		return m_wasResized;
	}

	bool Window::isIconified() const {
		return m_isIconified;
	}

}