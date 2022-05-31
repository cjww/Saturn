#pragma once
#include <stdint.h>

namespace sa {


	/*! @name Key and button actions
	*  @{ */
	enum class InputAction {
		/*! @brief The key or mouse button was released.
		*
		*  The key or mouse button was released.
		*
		*  @ingroup input
		*/
		RELEASE				= 0,
		/*! @brief The key or mouse button was pressed.
		*
		*  The key or mouse button was pressed.
		*
		*  @ingroup input
		*/
		PRESS				= 1,
		/*! @brief The key was held down until it repeated.
		*
		*  The key was held down until it repeated.
		*
		*  @ingroup input
		*/
		REPEAT				= 2
	};
	/*! @} */

	/*! @defgroup keys Keyboard keys
	 *  @brief Keyboard key IDs.
	 *
	 *  See [key input](@ref input_key) for how these are used.
	 *
	 *  These key codes are inspired by the _USB HID Usage Tables v1.12_ (p. 53-60),
	 *  but re-arranged to map to 7-bit ASCII for printable keys (function keys are
	 *  put in the 256+ range).
	 *
	 *  The naming of the key codes follow these rules:
	 *   - The US keyboard layout is used
	 *   - Names of printable alpha-numeric characters are used (e.g. "A", "R",
	 *     "3", etc.)
	 *   - For non-alphanumeric characters, Unicode:ish names are used (e.g.
	 *     "COMMA", "LEFT_SQUARE_BRACKET", etc.). Note that some names do not
	 *     correspond to the Unicode standard (usually for brevity)
	 *   - Keys that lack a clear US mapping are named "WORLD_x"
	 *   - For non-printable keys, custom names are used (e.g. "F4",
	 *     "BACKSPACE", etc.)
	 *
	 *  @ingroup input
	 *  @{
	 */
	enum class Key {
		UNKOWN = -1,
		/* Printable keys */
		SPACE = 32,
		/*@brief ' */
		APOSTROPHE = 39,
		/*@brief , */
		COMMA = 44,
		/*@brief - */
		MINUS = 45,
		/*@brief . */
		PERIOD = 46,
		/*@brief / */
		SLASH = 47,
		NUMBER_0 = 48,
		NUMBER_1 = 49,
		NUMBER_2 = 50,
		NUMBER_3 = 51,
		NUMBER_4 = 52,
		NUMBER_5 = 53,
		NUMBER_6 = 54,
		NUMBER_7 = 55,
		NUMBER_8 = 56,
		NUMBER_9 = 57,
		/*@brief ; */
		SEMICOLON = 59,
		/*@brief = */
		EQUAL = 61,
		A = 65,
		B = 66,
		C = 67,
		D = 68,
		E = 69,
		F = 70,
		G = 71,
		H = 72,
		I = 73,
		J = 74,
		K = 75,
		L = 76,
		M = 77,
		N = 78,
		O = 79,
		P = 80,
		Q = 81,
		R = 82,
		S = 83,
		T = 84,
		U = 85,
		V = 86,
		W = 87,
		X = 88,
		Y = 89,
		Z = 90,
		/*@brief [ */
		LEFT_BRACKET = 91,
		/*@brief \ */
		BACKSLASH = 92,
		/*@brief ] */
		RIGHT_BRACKET = 93,
		/*@brief ` */
		GRAVE_ACCENT = 96,
		/*@brief non-US #1 */
		WORLD_1 = 161,
		/*@brief non-US #2 */
		WORLD_2 = 162,

		/* Function keys */
		ESCAPE = 256,
		ENTER = 257,
		TAB = 258,
		BACKSPACE = 259,
		INSERT = 260,
		DELETE = 261,
		RIGHT = 262,
		LEFT = 263,
		DOWN = 264,
		UP = 265,
		PAGE_UP = 266,
		PAGE_DOWN = 267,
		HOME = 268,
		END = 269,
		CAPS_LOCK = 280,
		SCROLL_LOCK = 281,
		NUM_LOCK = 282,
		PRINT_SCREEN = 283,
		PAUSE = 284,
		F1 = 290,
		F2 = 291,
		F3 = 292,
		F4 = 293,
		F5 = 294,
		F6 = 295,
		F7 = 296,
		F8 = 297,
		F9 = 298,
		F10 = 299,
		F11 = 300,
		F12 = 301,
		F13 = 302,
		F14 = 303,
		F15 = 304,
		F16 = 305,
		F17 = 306,
		F18 = 307,
		F19 = 308,
		F20 = 309,
		F21 = 310,
		F22 = 311,
		F23 = 312,
		F24 = 313,
		F25 = 314,
		KP_0 = 320,
		KP_1 = 321,
		KP_2 = 322,
		KP_3 = 323,
		KP_4 = 324,
		KP_5 = 325,
		KP_6 = 326,
		KP_7 = 327,
		KP_8 = 328,
		KP_9 = 329,
		KP_DECIMAL = 330,
		KP_DIVIDE = 331,
		KP_MULTIPLY = 332,
		KP_SUBTRACT = 333,
		KP_ADD = 334,
		KP_ENTER = 335,
		KP_EQUAL = 336,
		LEFT_SHIFT = 340,
		LEFT_CONTROL = 341,
		LEFT_ALT = 342,
		LEFT_SUPER = 343,
		RIGHT_SHIFT = 344,
		RIGHT_CONTROL = 345,
		RIGHT_ALT = 346,
		RIGHT_SUPER = 347,
		MENU = 348,
		KEY_LAST = MENU
	};
	/*! @} */


	typedef uint32_t ModKeyFlags;
	/*! @defgroup mods Modifier key flags
	*  @brief Modifier key flags.
	*
	*  See [key input](@ref input_key) for how these are used.
	*
	*  @ingroup input
	*  @{ */
	enum class ModKeyFlagBits : ModKeyFlags {
		/*! @brief If this bit is set one or more Shift keys were held down.
		*
		*  If this bit is set one or more Shift keys were held down.
		*/
		SHIFT = 0x0001,
		/*! @brief If this bit is set one or more Control keys were held down.
		*
		*  If this bit is set one or more Control keys were held down.
		*/
		CONTROL = 0x0002,
		/*! @brief If this bit is set one or more Alt keys were held down.
		*
		*  If this bit is set one or more Alt keys were held down.
		*/
		ALT = 0x0004,
		/*! @brief If this bit is set one or more Super keys were held down.
		*
		*  If this bit is set one or more Super keys were held down.
		*/
		SUPER = 0x0008,
		/*! @brief If this bit is set the Caps Lock key is enabled.
		*
		*  If this bit is set the Caps Lock key is enabled and the @ref
		*  GLFW_LOCK_KEY_MODS input mode is set.
		*/
		CAPS_LOCK = 0x0010,
		/*! @brief If this bit is set the Num Lock key is enabled.
		*
		*  If this bit is set the Num Lock key is enabled and the @ref
		*  GLFW_LOCK_KEY_MODS input mode is set.
		*/
		NUM_LOCK = 0x0020

	};
	/*! @} */

	/*! @defgroup buttons Mouse buttons
	*  @brief Mouse button IDs.
	*/
	enum class MouseButton {
		BUTTON_1 = 0,
		BUTTON_2 = 1,
		BUTTON_3 = 2,
		BUTTON_4 = 3,
		BUTTON_5 = 4,
		BUTTON_6 = 5,
		BUTTON_7 = 6,
		BUTTON_8 = 7,
		LAST = BUTTON_8,
		LEFT = BUTTON_1,
		RIGHT = BUTTON_2,
		MIDDLE = BUTTON_3
	};

	/*! @defgroup joysticks Joysticks
	*  @brief Joystick IDs.
	*
	*  See [joystick input](@ref joystick) for how these are used.
	*
	*  @ingroup input
	*  @{ */
	enum class Joystick {
		JOYSTICK_1 = 0,
		JOYSTICK_2 = 1,
		JOYSTICK_3 = 2,
		JOYSTICK_4 = 3,
		JOYSTICK_5 = 4,
		JOYSTICK_6 = 5,
		JOYSTICK_7 = 6,
		JOYSTICK_8 = 7,
		JOYSTICK_9 = 8,
		JOYSTICK_10 = 9,
		JOYSTICK_11 = 10,
		JOYSTICK_12 = 11,
		JOYSTICK_13 = 12,
		JOYSTICK_14 = 13,
		JOYSTICK_15 = 14,
		JOYSTICK_16 = 15,
		JOYSTICK_LAST = JOYSTICK_16
	};
	/*! @} */

	/*! @defgroup hat_state Joystick hat states
	 *  @brief Joystick hat states.
	 *
	 *  See [joystick hat input](@ref joystick_hat) for how these are used.
	 *
	 *  @ingroup input
	 *  @{ */
	enum class JoystickHat {

		CENTERED           = 0,
		UP                 = 1,
		RIGHT              = 2,
		DOWN               = 4,
		LEFT               = 8,
		RIGHT_UP           = RIGHT | UP,
		RIGHT_DOWN         = RIGHT | DOWN,
		LEFT_UP            = LEFT  | UP,
		LEFT_DOWN          = LEFT  | DOWN,
	};
	/*! @} */


	/*! @defgroup gamepad_buttons Gamepad buttons
	*  @brief Gamepad buttons.
	*
	*  See @ref gamepad for how these are used.
	*
	*  @ingroup input
	*  @{ */
	enum class GamepadButtons {

		A = 0,
		B = 1,
		X = 2,
		Y = 3,
		LEFT_BUMPER = 4,
		RIGHT_BUMPER = 5,
		BACK = 6,
		START = 7,
		GUIDE = 8,
		LEFT_THUMB = 9,
		RIGHT_THUMB = 10,
		DPAD_UP = 11,
		DPAD_RIGHT = 12,
		DPAD_DOWN = 13,
		DPAD_LEFT = 14,
		LAST = DPAD_LEFT,

		CROSS = A,
		CIRCLE = B,
		SQUARE = X,
		TRIANGLE = Y
	};
	/*! @} */
	/*! @defgroup gamepad_axes Gamepad axes
	*  @brief Gamepad axes.
	*
	*  See @ref gamepad for how these are used.
	*
	*  @ingroup input
	*  @{ */
	enum class GamepadAxis {
		LEFT_X        = 0,
		LEFT_Y        = 1,
		RIGHT_X       = 2,
		RIGHT_Y       = 3,
		LEFT_TRIGGER  = 4,
		RIGHT_TRIGGER = 5,
		LAST          = RIGHT_TRIGGER
	};
	/*! @} */
}