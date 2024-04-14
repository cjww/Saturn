#pragma once


#include "structs.hpp"

#include "InputEnums.hpp"
#include "Image.hpp"

#include <set>
#include <array>
#include "Resources/ResourceManager.hpp"

struct GLFWwindow;
struct GLFWmonitor;


namespace sa {

	enum class StandardCursor {
		ARROW		= 0x00036001,
		IBEAM		= 0x00036002,
		CROSSHAIR	= 0x00036003,
		HAND		= 0x00036004,
		HRESIZE		= 0x00036005,
		VRESIZE		= 0x00036006
	};

	enum class ConnectionState {
		CONNECTED,
		DISCONNECTED
	};

	struct GamepadState {
		std::array<bool, 15> buttons;
		std::array<float, 6> axes;

		bool getButton(GamepadButton button) { return buttons[(int)button]; }
		float getAxis(GamepadAxis axis) { return axes[(int)axis]; };
	};

	/*
	* @param[in] key The key involved in the event.
	* @param[in] inputAction The action that caused the event.
	* @param[in] modKeyflags Modification flags indicating specific key states, i.e. if shift was held or num lock is enabled.
	* @param[in] scancode The system-specific scancode of the key.
	*/
	typedef std::function<void(Key, InputAction, ModKeyFlags, int)> KeyCallback;
	/*
	* @param[in] button The mouse button involved in the event.
	* @param[in] inputAction The action that caused the event.
	* @param[in] modKeyflags Modification flags indicating specific key states, i.e. if shift was held or num lock is enabled.
	*/
	typedef std::function<void(MouseButton, InputAction, ModKeyFlags)> MouseButtonCallback;
	/// @param[in] x, y The scroll wheel difference.
	typedef std::function<void(double, double)> ScrollCallback;
	/*
	* @param[in] joystick The ID of the joystick.
	* @param[in] connectionState The connection state of the joystick, i.e. if it is connected or disconnected.
	*/
	typedef std::function<void(Joystick, ConnectionState)> JoystickConnectedCallback;
	/*
	* @param[in] count Number of strings in paths
	* @param[in] paths Array of strings containing paths to dropped files
	*/
	typedef std::function<void(int, const char**)> DragDropCallback;
	/// @param[in] focus True of focus was gained and False if focus was lost
	typedef std::function<void(bool)> FocusCallback;


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

		std::vector<KeyCallback> m_onKeyFunctions;
		std::vector<MouseButtonCallback> m_onMouseButtonFunctions;
		std::vector<ScrollCallback> m_onScrollFunctions;
		std::vector<DragDropCallback> m_onDragDropFunctions;
		std::vector<FocusCallback> m_onFocusFunctions;

		
	protected:
		static void onResize(GLFWwindow* window, int width, int height);
		static void onIconify(GLFWwindow* window, int iconified);
		static void onKey(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void onMouseButton(GLFWwindow* window, int button, int action, int mods);
		static void onScroll(GLFWwindow* window, double x, double y);
		static void onClose(GLFWwindow* window);
		static void onDragDrop(GLFWwindow* window, int count, const char** pathUTF8);
		static void onFocus(GLFWwindow* window, int hasGainedFocus);


		static void onJoystickDetect(int jid, int state);

		virtual void create(uint32_t width, uint32_t height, const char* title, GLFWmonitor* monitor);
		virtual void shutDown();

		void setWasResized(bool value);
	public:
		static void SetJoystickConnectedCallback(JoystickConnectedCallback func);
		static bool IsGamepad(Joystick joystick);
		static bool IsJoystickPresent(Joystick joystick);
		static std::string GetJoystickName(Joystick joystick);
		static GamepadState GetGamepadState(Joystick joystick);
		static float GetGamepadAxisDeadzone();
		static void SetGamepadAxisDeadzone(float deadzone);

		static ResourceID CreateCursor(const Image& image, Offset hot = { 0, 0 });
		static ResourceID CreateCursor(StandardCursor cursor);
		
		
		Window(uint32_t width, uint32_t height, const char* title);
		Window(uint32_t monitorIndex);

		virtual ~Window();

		bool isOpen();
		void pollEvents();
		void waitEvents(float timeoutSeconds = -1.f);

		void close();

		void setRefreshRate(int rate);
		int getRefreshRate() const;

		void setMonitor(int monitorIndex);
		int getCurrentMonitor() const;
		int getPrimaryMonitor() const;
		int getMonitorCount() const;

		void setFullscreen(bool fullscreen);
		void toggleFullscreen();
		bool isFullscreen() const;

		void setBorderless(bool isBorderless);
		void toggleBorderless();
		bool isBorderless() const;
		
		void setBorderlessFullscreen(bool value);
		void toggleBorderlessFullscreen();

		void setResizable(bool isResizable);
		bool isResizable() const;

		void setWindowTitle(const char* title);
		void setWindowTitle(const std::string& title);
		void setWindowTitle(std::string&& title);

		Extent getCurrentExtent() const;
		Offset getWindowPosition() const;

		GLFWwindow* getWindowHandle() const;
		void* getWin32WindowHandle() const;

		int getKey(Key keyCode) const;
		int getMouseButton(MouseButton button) const;

		Point getCursorPosition() const;
		void setCursorPosition(const Point& position);

		void setCursorHidden(bool value);
		bool isCursorHidden() const;

		void setCursorDisabled(bool value);
		bool isCursorDisabled() const;

		void setCursor(ResourceID cursor);

		void addKeyCallback(KeyCallback func);
		void addMouseButtonCallback(MouseButtonCallback func);
		void addScrollCallback(ScrollCallback func);
		void addDragDropCallback(DragDropCallback func);
		void addFocusCallback(FocusCallback func);

		bool wasResized() const;
		bool isIconified() const;

	};
}