#include "pch.h"
#include "Window.hpp"

#include "GLFW/glfw3.h"

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#define NOMINMAX
#include "GLFW/glfw3native.h"
#endif // _WIN32



namespace sa {

	void Window::onResize(GLFWwindow* window, int width, int height) {
		Window* thisWindow = (Window*)glfwGetWindowUserPointer(window);
		if (thisWindow->isIconified()) {
			return;
		}

		if (!thisWindow->isResizable()) {
			return;
		}

		Extent extent = { width, height };
		if (thisWindow->m_windowedExtent.width != extent.width || thisWindow->m_windowedExtent.height != height) {
			thisWindow->m_wasResized = true;
			thisWindow->m_windowedExtent = extent;
			SA_DEBUG_LOG_INFO("Window: Window resize ", width, height);
		}
	}

	void Window::onIconify(GLFWwindow* window, int iconified) {
		Window* thisWindow = (Window*)glfwGetWindowUserPointer(window);
		thisWindow->m_isIconified = iconified;
		SA_DEBUG_LOG_WARNING("Window: Window iconified");

	}

	void Window::onKey(GLFWwindow* window, int key, int scancode, int action, int mods) {
		Window* thisWindow = (Window*)glfwGetWindowUserPointer(window);
	#ifdef _WIN32
		if (key == GLFW_KEY_F11 && action == GLFW_PRESS) {
			thisWindow->toggleFullscreen();
		}
		if (key == GLFW_KEY_F10 && action == GLFW_PRESS) {
			thisWindow->toggleBorderless();
			//thisWindow->toggleBorderlessFullscreen();
		}
	#endif // _WIN32

		for (auto& func : thisWindow->m_onKeyFunctions) {
			func((Key)key, (InputAction)action, (ModKeyFlags)mods, scancode);
		}
	}

	void Window::onMouseButton(GLFWwindow* window, int button, int action, int mods) {
		Window* thisWindow = (Window*)glfwGetWindowUserPointer(window);
		for (auto& func : thisWindow->m_onMouseButtonFunctions) {
			func((MouseButton)button, (InputAction)action, (ModKeyFlags)mods);
		}
	}

	void Window::onScroll(GLFWwindow* window, double x, double y) {
		Window* thisWindow = (Window*)glfwGetWindowUserPointer(window);
		for (auto& func : thisWindow->m_onScrollFunctions) {
			func(x, y);
		}
	}

	void Window::onClose(GLFWwindow* window) {
		Window* w = (Window*)glfwGetWindowUserPointer(window);
		w->close();
	}

	void Window::onDragDrop(GLFWwindow* window, int count, const char** pathUTF8) { 
		Window* thisWindow = (Window*)glfwGetWindowUserPointer(window);
		for (auto& func : thisWindow->m_onDragDropFunctions) {
			func(count, pathUTF8);
		}
	}

	void Window::onFocus(GLFWwindow* window, int hasGainedFocus) {
		Window* thisWindow = (Window*)glfwGetWindowUserPointer(window);
		for (auto& func : thisWindow->m_onFocusFunctions) {
			func(hasGainedFocus);
		}
	}

	void Window::onJoystickDetect(int jid, int state) {
		if (s_onJoystickDetectFunction) {
			if (state == GLFW_CONNECTED) {	
				s_onJoystickDetectFunction((Joystick)jid, ConnectionState::CONNECTED);
			}
			else if (state == GLFW_DISCONNECTED) {
				s_onJoystickDetectFunction((Joystick)jid, ConnectionState::DISCONNECTED);
			}
		}
	}


	void Window::create(uint32_t width, uint32_t height, const char* title, GLFWmonitor* monitor) {
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		m_monitor = monitor;
		if (m_monitor) {
			int refreshrate = glfwGetVideoMode(m_monitor)->refreshRate;
			glfwWindowHint(GLFW_REFRESH_RATE, refreshrate);
		}
		m_window = glfwCreateWindow(width, height, title, m_monitor, nullptr);
		if (!m_window) {
			throw std::runtime_error("Failed to create GLFW window");
		}

		glfwSetWindowSizeCallback(m_window, &Window::onResize);
		glfwSetWindowIconifyCallback(m_window, &Window::onIconify);
		glfwSetKeyCallback(m_window, &Window::onKey);
		glfwSetMouseButtonCallback(m_window, &Window::onMouseButton);
		glfwSetScrollCallback(m_window, &Window::onScroll);
		glfwSetWindowCloseCallback(m_window, &Window::onClose);
		glfwSetDropCallback(m_window, &Window::onDragDrop);
		glfwSetWindowFocusCallback(m_window, &Window::onFocus);
		
		glfwSetJoystickCallback(&Window::onJoystickDetect);

		glfwSetWindowUserPointer(m_window, this);

		Extent extent = { width, height };
		m_windowedExtent = extent;

		m_isIconified = false;
		m_wasResized = false;

		
	}

	void Window::shutDown() {
		glfwDestroyWindow(m_window);
		m_window = nullptr;
	}

	void Window::SetJoystickConnectedCallback(JoystickConnectedCallback func) {
		s_onJoystickDetectFunction = func;
	}

	bool Window::IsGamepad(Joystick joystick) {
		return glfwJoystickIsGamepad((int)joystick) == GLFW_TRUE;
	}

	bool Window::IsJoystickPresent(Joystick joystick) {
		return glfwJoystickPresent((int)joystick);
	}
	
	std::string Window::GetJoystickName(Joystick joystick) {
		return glfwGetJoystickName((int)joystick);
	}

	GamepadState Window::GetGamepadState(Joystick joystick) {
		GLFWgamepadstate state;
		GamepadState gamepadState;
		if (!glfwGetGamepadState((int)joystick, &state)) {
			SA_DEBUG_LOG_WARNING("Joystick ", (int)joystick, " was not a gamepad or not present");
			return {};
		}
		for (int i = 0; i < 15; i++) {
			gamepadState.buttons[i] = state.buttons[i];
		}
		for (int i = 0; i < 6; i++) {
			if (abs(state.axes[i]) < s_gamepadAxisDeadzone) {
				gamepadState.axes[i] = 0;
				continue;
			}
			gamepadState.axes[i] = state.axes[i];
		}
		return gamepadState;
	}

	float Window::GetGamepadAxisDeadzone() {
		return s_gamepadAxisDeadzone;
	}

	void Window::SetGamepadAxisDeadzone(float deadzone) {
		s_gamepadAxisDeadzone = deadzone;
	}

	ResourceID Window::CreateCursor(const Image& image, Offset hot) {
		GLFWimage gImage;
		gImage.width = image.getWidth();
		gImage.height = image.getHeight();
		gImage.pixels = image.getPixels();
		GLFWcursor* pCursor = glfwCreateCursor(&gImage, hot.x, hot.y);
		if (!pCursor) {
			return NULL_RESOURCE;
		}
		return ResourceManager::get().insert<GLFWcursor*>(pCursor);
	}

	ResourceID Window::CreateCursor(StandardCursor cursor) {
		GLFWcursor* pCursor = glfwCreateStandardCursor((int)cursor);
		if (!pCursor) {
			return NULL_RESOURCE;
		}
		return ResourceManager::get().insert<GLFWcursor*>(pCursor);
	}

	Window::Window(uint32_t width, uint32_t height, const char* title) {
		if (s_windowCount == 0) {
			if (!glfwInit()) {
				throw std::runtime_error("Failed to initialize GLFW!");
			}
			ResourceManager::get().setCleanupFunction<GLFWcursor*>([](GLFWcursor** p) { glfwDestroyCursor(*p); });
		}
		s_windowCount++;
		create(width, height, title, nullptr);
	}

	Window::Window(uint32_t monitorIndex) {
		if (s_windowCount == 0) {
			if (!glfwInit()) {
				throw std::runtime_error("Failed to initialize GLFW!");
			}
			ResourceManager::get().setCleanupFunction<GLFWcursor*>([](GLFWcursor** p) { glfwDestroyCursor(*p); });
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
	}

	Window::~Window() {
		if (m_window != nullptr) {
			shutDown();

			s_windowCount--;
			if (s_windowCount == 0) {
				ResourceManager::get().clearContainer<GLFWcursor*>();
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

	void Window::waitEvents(float timeoutSeconds) {
		if (timeoutSeconds < 0) {
			glfwWaitEvents();
			return;
		}
		glfwWaitEventsTimeout(timeoutSeconds);
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

	int Window::getRefreshRate() const {
		GLFWmonitor* monitor = m_monitor;
		if (!monitor) {
			int count;
			GLFWmonitor** monitors = glfwGetMonitors(&count);
			monitor = monitors[getCurrentMonitor()];
		}
		const GLFWvidmode* vidmode = glfwGetVideoMode(monitor);
		return vidmode->refreshRate;

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

	int Window::getCurrentMonitor() const {
		int count;
		GLFWmonitor** monitors = glfwGetMonitors(&count);
		
		int window_xpos, window_ypos;
		glfwGetWindowPos(m_window, &window_xpos, &window_ypos);

		for (int i = 0; i < count; i++) {
			int xpos, ypos, width, height;
			glfwGetMonitorWorkarea(monitors[i], &xpos, &ypos, &width, &height);

			if (window_xpos > xpos && window_xpos < xpos + width) {	
				if (window_ypos > ypos && window_ypos < ypos + height) {
					return i;
				}
			}
		}
		return -1;
	}

	int Window::getPrimaryMonitor() const {
		int count;
		GLFWmonitor** pMonitors = glfwGetMonitors(&count);
		GLFWmonitor* pMonitor = glfwGetPrimaryMonitor();
		for (int i = 0; i < count; i++) {
			if (pMonitor == pMonitors[i]) {
				return i;
			}
		}
		return -1;
	}

	int Window::getMonitorCount() const {
		int count;
		glfwGetMonitors(&count);
		return count;
	}

	void Window::setFullscreen(bool fullscreen) {
		if (!isResizable())
			return;

		if (fullscreen) {
			setMonitor(getCurrentMonitor());
		}
		else {
			setMonitor(-1);
		}
	}

	void Window::toggleFullscreen() {
		setFullscreen(!isFullscreen());
	}

	bool Window::isFullscreen() const {
		return m_monitor != nullptr;
	}

	void Window::setBorderless(bool isBorderless) {
		glfwSetWindowAttrib(m_window, GLFW_DECORATED, !isBorderless);
	}

	void Window::toggleBorderless() {
		setBorderless(!isBorderless());
	}

	bool Window::isBorderless() const {
		return !(bool)glfwGetWindowAttrib(m_window, GLFW_DECORATED);
	}

	void Window::setBorderlessFullscreen(bool value) {
		if (!isResizable())
			return;
		setBorderless(value);
		if (value) {
			int count;
			GLFWmonitor** monitors = glfwGetMonitors(&count);
			GLFWmonitor* monitor = monitors[getCurrentMonitor()];

			int xpos, ypos, width, height;
		
			glfwGetMonitorWorkarea(monitor, &xpos, &ypos, &width, &height);
			const GLFWvidmode* vidmode = glfwGetVideoMode(monitor);
			glfwSetWindowMonitor(m_window, nullptr, xpos, ypos, width, height, vidmode->refreshRate);
		}
		else {
			glfwSetWindowMonitor(m_window, nullptr, 0, 0, m_windowedExtent.width, m_windowedExtent.height, GLFW_DONT_CARE);
		}
	}

	void Window::toggleBorderlessFullscreen() {
		setBorderlessFullscreen(!isBorderless());
	}

	void Window::setResizable(bool isResizable) {
		glfwSetWindowAttrib(m_window, GLFW_RESIZABLE, isResizable);
	}

	bool Window::isResizable() const {
		return glfwGetWindowAttrib(m_window, GLFW_RESIZABLE);
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

	Offset Window::getWindowPosition() const {
		Offset offset;
		glfwGetWindowPos(m_window, &offset.x, &offset.y);
		return offset;
	}

	GLFWwindow* Window::getWindowHandle() const {
		return m_window;
	}

	void* Window::getWin32WindowHandle() const {
#ifdef _WIN32
		return glfwGetWin32Window(m_window);
#else
		return 0;
#endif // _WIN32
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

	void Window::setCursorHidden(bool value) {
		glfwSetInputMode(m_window, GLFW_CURSOR, (value) ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);
	}

	bool Window::isCursorHidden() const {
		return glfwGetInputMode(m_window, GLFW_CURSOR) == GLFW_CURSOR_HIDDEN;
	}

	void Window::setCursorDisabled(bool value) {
		glfwSetInputMode(m_window, GLFW_CURSOR, (value) ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
	}

	bool Window::isCursorDisabled() const {
		return glfwGetInputMode(m_window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;
	}

	void Window::setCursor(ResourceID cursor) {
		if (cursor == NULL_RESOURCE) {
			glfwSetCursor(m_window, NULL);
			return;
		}
		GLFWcursor** pCursor = ResourceManager::get().get<GLFWcursor*>(cursor);
		if (!pCursor) {
			SA_DEBUG_LOG_WARNING("Tried to set invalid cursor");
			return;
		}
		glfwSetCursor(m_window, *pCursor);
	}

	void Window::addKeyCallback(KeyCallback func) {
		m_onKeyFunctions.push_back(func);
	}

	void Window::addScrollCallback(ScrollCallback func) {
		m_onScrollFunctions.push_back(func);
	}

	void Window::addDragDropCallback(DragDropCallback func) {
		m_onDragDropFunctions.push_back(func);
	}

	void Window::addFocusCallback(FocusCallback func) {
		m_onFocusFunctions.push_back(func);
	}

	void Window::addMouseButtonCallback(MouseButtonCallback func) {
		m_onMouseButtonFunctions.push_back(func);
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