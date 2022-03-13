#pragma once

#include <functional>
#include <stdexcept>

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

struct GLFWwindow;
struct GLFWmonitor;
namespace sa {

	class Window {
	private:

		GLFWwindow* m_window;
		GLFWmonitor* m_monitor;

		glm::ivec2 m_windowedExtent;

		bool m_isIconified;
		bool m_wasResized;

		std::function<void(int, int, int, int)> m_onKeyFunction;
		std::function<void(int, int, int)> m_onMouseButtonFunction;

		static void onResize(GLFWwindow* window, int width, int height);
		static void onIconify(GLFWwindow* window, int iconified);
		static void onKey(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void onMouseButton(GLFWwindow* window, int button, int action, int mods);
		static void onClose(GLFWwindow* window);

		void create(uint32_t width, uint32_t height, const char* title, GLFWmonitor* monitor);
	
		void shutDown();

		static unsigned int windowCount;
	public:

		Window(uint32_t width, uint32_t height, const char* title);
		Window(uint32_t monitorIndex);

		virtual ~Window();

		bool isOpen();
		void pollEvents();

		void close();

		void setRefreshRate(int rate);
		void setMonitor(int monitorIndex);
		void toggleFullscreen();

		void setWindowTitle(const char* title);
		void setWindowTitle(const std::string& title);
		void setWindowTitle(std::string&& title);

		glm::ivec2 getCurrentExtent() const;

		GLFWwindow* getWindowHandle() const;

		int getKey(int keyCode) const;
		int getMouseButton(int button) const;

		glm::vec2 getCursorPosition() const;
		void setCursorPosition(const glm::vec2& position);

		void setHideCursor(bool value);
		bool isHidingCursor() const;

		void setKeyCallback(std::function<void(int, int, int, int)> func);

		void setMouseButtonCallback(std::function<void(int, int, int)> func);

		void setWasResized(bool value);
		bool wasResized() const;
		bool isIconified() const;

	};

	struct RenderWindowProxy {
		static glm::vec2 getCurrentExtent(const RenderWindow* window) {
			return window->getCurrentExtent();
		}
	};
}