#pragma once

#include <InlineMath.hpp>
#include <cstdint>
#undef DELETE // hogy a faszomat húzzam bele a winapiba


namespace inl {


// Key states
enum class eKeyState {
	DOWN,
	UP,
	DOUBLE,
};


// Mouse click
enum class eMouseButton {
	LEFT,
	MIDDLE,
	RIGHT,
	EXTRA1,
	EXTRA2,
	COUNT,
};

struct MouseButtonEvent {
	int32_t x;
	int32_t y;
	eMouseButton button;
	eKeyState state;
};



// Mouse move
struct MouseMoveEvent {
	int32_t relx;
	int32_t rely;
	int32_t absx;
	int32_t absy;
};


struct MouseWheelEvent {
	float rotation;
};


// Keyboard
enum class eKey : unsigned {
	NUMBER_0 = 0x30,
	NUMBER_1,
	NUMBER_2,
	NUMBER_3,
	NUMBER_4,
	NUMBER_5,
	NUMBER_6,
	NUMBER_7,
	NUMBER_8,
	NUMBER_9,

	A = 0x41,
	B,
	C,
	D,
	E,
	F,
	G,
	H,
	I,
	J,
	K,
	L,
	M,
	N,
	O,
	P,
	Q,
	R,
	S,
	T,
	U,
	V,
	W,
	X,
	Y,
	Z,

	NUMPAD_0 = 0x60,
	NUMPAD_1,
	NUMPAD_2,
	NUMPAD_3,
	NUMPAD_4,
	NUMPAD_5,
	NUMPAD_6,
	NUMPAD_7,
	NUMPAD_8,
	NUMPAD_9,

	F1 = 0x70,
	F2,
	F3,
	F4,
	F5,
	F6,
	F7,
	F8,
	F9,
	F10,
	F11,
	F12,
	F13,
	F14,
	F15,
	F16,
	F17,
	F18,
	F19,
	F20,
	F21,
	F22,
	F23,
	F24,

	BACKSPACE,
	TAB,
	CLEAR,
	ENTER,
	CAPS_LOCK,
	NUM_LOCK,
	SCROLL_LOCK,
	PAUSE,
	ESCAPE,
	SPACE,
	PAGE_UP,
	PAGE_DOWN,
	END,
	HOME,
	LEFT,
	UP,
	RIGHT,
	DOWN,
	SELECT,
	PRINT,
	PRINT_SCREEN,
	INSERT,
	DELETE,
	SLEEP,
	WINKEY_LEFT,
	WINKEY_RIGHT,

	MULTIPLY,
	ADD,
	SUBTRACT,
	DIVIDE,
	DECIMAL,
	SPEPARATOR,

	SHIFT_LEFT,
	SHIFT_RIGHT,
	CONTROL_LEFT,
	CONTROL_RIGHT,
	ALT_LEFT,
	ALT_RIGHT,

	UNKNOWN,
	COUNT,
};

struct KeyboardEvent {
	eKeyState state;
	eKey key;
	int repcount = 0;
};

enum class eResizeMode
{
	RESTORED,
	MINIMIZED,
	MAXIMIZED,
	MAXSHOW,
	MAXHIDE,
};

struct ResizeEvent {
	Vec2u size;
	Vec2u clientSize;
	eResizeMode resizeMode;
};

// Joystick click
enum class eJoystickButton {
	BUTTON0,
	BUTTON1,
	BUTTON2,
	BUTTON3,
	BUTTON4,
	BUTTON5,
	BUTTON6,
	BUTTON7,
	BUTTON8,
	BUTTON9,
};

struct JoystickButtonEvent {
	eJoystickButton button;
	eKeyState state;
};

// Joystick move
enum class eJoystickAxis {
	AXIS0,
	AXIS1,
	AXIS2,
	AXIS3,
	AXIS4,
	AXIS5,
};

struct JoystickMoveEvent {
	int axis;
	float absPos;
};


// Union for all events
enum class eInputEventType {
	MOUSE_BUTTON,
	MOUSE_MOVE,
	KEYBOARD,
	JOYSTICK_BUTTON,
	JOYSTICK_MOVE,
	INVALID,
};

struct InputEvent {
	InputEvent() : type(eInputEventType::INVALID) {}
	explicit InputEvent(const MouseButtonEvent& evt) : type(eInputEventType::MOUSE_BUTTON), mouseButton(evt) {}
	explicit InputEvent(const MouseMoveEvent& evt) : type(eInputEventType::MOUSE_MOVE), mouseMove(evt) {}
	explicit InputEvent(const KeyboardEvent& evt) : type(eInputEventType::KEYBOARD), keyboard(evt) {}
	explicit InputEvent(const JoystickButtonEvent& evt) : type(eInputEventType::JOYSTICK_BUTTON), joystickButton(evt) {}
	explicit InputEvent(const JoystickMoveEvent& evt) : type(eInputEventType::JOYSTICK_MOVE), joystickMove(evt) {}
	eInputEventType type;
	union {
		MouseButtonEvent mouseButton;
		MouseMoveEvent mouseMove;
		KeyboardEvent keyboard;
		JoystickButtonEvent joystickButton;
		JoystickMoveEvent joystickMove;
	};
};




} // namespace inl