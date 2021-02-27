#pragma once
#include "common.hpp"

#include <functional>
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "Renderer.hpp"

namespace NAME_SPACE {

	class RenderWindow {
	private:

		GLFWwindow* m_window;
		GLFWmonitor* m_monitor;
		VkSurfaceKHR m_surface;
		VkExtent2D m_windowedExtent;

		bool m_isIconified;
		bool m_needsRecreation;

		std::function<void(int, int, int, int)> m_onKeyFunction;
		std::function<void(int, int, int)> m_onMouseButtonFunction;

		static void onResize(GLFWwindow* window, int width, int height);
		static void onIconify(GLFWwindow* window, int iconified);
		static void onKey(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void onMouseButton(GLFWwindow* window, int button, int action, int mods);
		static void onClose(GLFWwindow* window);

		void create(uint32_t width, uint32_t height, const char* title, GLFWmonitor* monitor);
		void recreate(VkExtent2D newExtent);

		void shutDown();

		uint32_t getNextFrameImage();
	public:

		RenderWindow(uint32_t width, uint32_t height, const char* title);
		RenderWindow(uint32_t monitorIndex);

		virtual ~RenderWindow();

		bool isOpen();
		void pollEvents();

		void beginFrame();
		void endFrame();
		
		void close();

		void setRefreshRate(int rate);
		void setMonitor(int monitorIndex);
		void toggleFullscreen();

		void setWindowTitle(const char* title);
		void setWindowTitle(const std::string& title);
		void setWindowTitle(std::string&& title);

		VkExtent2D getCurrentExtent() const;

		GLFWwindow* getWindowHandle() const;

		int getKey(int keyCode) const;
		int getMouseButton(int button) const;

		glm::vec2 getCursorPosition() const;
		void setCursorPosition(const glm::vec2& position);

		void setHideCursor(bool value);
		bool isHidingCursor() const;

		void setKeyCallback(std::function<void(int, int, int, int)> func);

		void setMouseButtonCallback(std::function<void(int, int, int)> func);

	};

	struct RenderWindowProxy {
		static glm::vec2 getCurrentExtent(const RenderWindow* window) {
			VkExtent2D extent = window->getCurrentExtent();
			return glm::vec2(extent.width, extent.height);
		}
	};
}