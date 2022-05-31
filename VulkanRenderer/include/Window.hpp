#pragma once


#include "structs.hpp"

#include "InputEnums.hpp."

#include <set>

struct GLFWwindow;
struct GLFWmonitor;


namespace sa {

	enum class ConnectionState {
		CONNECTED,
		DISCONNECTED
	};

	struct GamepadState {
		bool buttons[15];
		float axes[6];
	};

	typedef std::function<void(Key, InputAction, ModKeyFlags, int)> KeyCallback;
	typedef std::function<void(MouseButton, InputAction, ModKeyFlags)> MouseButtonCallback;
	typedef std::function<void(Joystick, ConnectionState)> JoystickConnectedCallback;

	class Window {
	private:
		inline static JoystickConnectedCallback s_onJoystickDetectFunction;
		inline static unsigned int s_windowCount = 0;
		inline static float s_gamepadAxisDeadzone = 0.2f;

		GLFWwindow* m_window;
		GLFWmonitor* m_monitor;

		Extent m_windowedExtent;

		bool m_isIconified;
		bool m_wasResized;

		KeyCallback m_onKeyFunction;
		MouseButtonCallback m_onMouseButtonFunction;

	protected:
		static void onResize(GLFWwindow* window, int width, int height);
		static void onIconify(GLFWwindow* window, int iconified);
		static void onKey(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void onMouseButton(GLFWwindow* window, int button, int action, int mods);
		static void onClose(GLFWwindow* window);
		static void onJoystickDetect(int jid, int state);

		virtual void create(uint32_t width, uint32_t height, const char* title, GLFWmonitor* monitor);
		virtual void shutDown();
	
	public:
		static void SetJoystickConnectedCallback(JoystickConnectedCallback func);
		static bool IsGamepad(Joystick joystick);
		static GamepadState GetGamepadState(Joystick joystick);
		static float GetGamepadAxisDeadzone();
		static void SetGamepadAxisDeadzone(float deadzone);

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

		Extent getCurrentExtent() const;

		GLFWwindow* getWindowHandle() const;

		int getKey(Key keyCode) const;
		int getMouseButton(MouseButton button) const;

		Point getCursorPosition() const;
		void setCursorPosition(const Point& position);

		void setCursorHidden(bool value);
		bool isCursorHidden() const;

		void setCursorDisabled(bool value);
		bool isCursorDisabled() const;


		void setKeyCallback(KeyCallback func);
		void setMouseButtonCallback(MouseButtonCallback func);

		void setWasResized(bool value);
		bool wasResized() const;
		bool isIconified() const;

	};
}