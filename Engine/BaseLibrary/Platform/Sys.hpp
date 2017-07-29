// Static class that encapsulates the Operating System.
#pragma once

#include <string>
#include "PlatformDefinitions.h"
#include "..\Common.hpp"

enum class eKey
{
	A,				///< The A key
	B,				///< The B key
	C,				///< The C key
	D,				///< The D key
	E,				///< The E key
	F,				///< The F key
	G,				///< The G key
	H,				///< The H key
	I,				///< The I key
	J,				///< The J key
	K,				///< The K key
	L,				///< The L key
	M,				///< The M key
	N,				///< The N key
	O,				///< The O key
	P,				///< The P key
	Q,				///< The Q key
	R,				///< The R key
	S,				///< The S key
	T,				///< The T key
	U,				///< The U key
	V,				///< The V key
	W,				///< The W key
	X,				///< The X key
	Y,				///< The Y key
	Z,				///< The Z key
	NUM0,			///< The 0 key
	NUM1,			///< The 1 key
	NUM2,			///< The 2 key
	NUM3,			///< The 3 key
	NUM4,			///< The 4 key
	NUM5,			///< The 5 key
	NUM6,			///< The 6 key
	NUM7,			///< The 7 key
	NUM8,			///< The 8 key
	NUM9,			///< The 9 key
	ESC,			///< The Escape key
	LEFT_CONTROL,			///< The left Control key
	LSHIFT,			///< The left Shift key
	LEFT_ALT,			///< The left Alt key
	LSYS,			///< The left OS specific key: window (Windows and Linux), apple (MacOS X), ...
	RIGHT_CONTROL,			///< The right Control key
	RSHIFT,			///< The right Shift key
	RIGHT_ALT,			///< The right Alt key
	RSYS,			///< The right OS specific key: window (Windows and Linux), apple (MacOS X), ...
	MENU,			///< The Menu key
	LBRACKET,		///< The [ key
	RBRACKET,		///< The ] key
	SEMICOLON,		///< The ; key
	COMMA,			///< The , key
	PERIOD,			///< The . key
	QUOTE,			///< The ' key
	SLASH,			///< The / key
	BACKSLASH,		///< The \ key
	TILDE,			///< The ~ key
	EQUAL,			///< The = key
	DASH,			///< The - key
	SPACE,			///< The Space key
	ENTER,			///< The Return key
	BACKSPACE,		///< The Backspace key
	TAB,			///< The Tabulation key
	PAGEUP,			///< The Page up key
	PAGEDDOWN,		///< The Page down key
	END,			///< The End key
	HOME,			///< The Home key
	INS,			///< The Insert key
	DEL,			///< The Delete key
	ADD,			///< The + key
	SUB,			///< The - key
	MUL,			///< The * key
	DIV,			///< The / key
	LEFT_ARROW,		///< Left arrow
	RIGHT_ARROW,	///< Right arrow
	UP,				///< Up arrow
	DOWN,			///< Down arrow
	NUMPAD0,		///< The numpad 0 key
	NUMPAD1,		///< The numpad 1 key
	NUMPAD2,		///< The numpad 2 key
	NUMPAD3,		///< The numpad 3 key
	NUMPAD4,		///< The numpad 4 key
	NUMPAD5,		///< The numpad 5 key
	NUMPAD6,		///< The numpad 6 key
	NUMPAD7,		///< The numpad 7 key
	NUMPAD8,		///< The numpad 8 key
	NUMPAD9,		///< The numpad 9 key
	F1,				///< The F1 key
	F2,				///< The F2 key
	F3,				///< The F3 key
	F4,				///< The F4 key
	F5,				///< The F5 key
	F6,				///< The F6 key
	F7,				///< The F7 key
	F8,				///< The F8 key
	F9,				///< The F9 key
	F10,			///< The F10 key
	F11,			///< The F11 key
	F12,			///< The F12 key
	F13,			///< The F13 key
	F14,			///< The F14 key
	F15,			///< The F15 key
	F16,			///< The F16 key
	F17,			///< The F17 key
	F18,			///< The F18 key
	F19,			///< The F19 key
	F20,			///< The F20 key
	F21,			///< The F21 key
	F22,			///< The F22 key
	F23,			///< The F23 key
	F24,			///< The F24 key
	PAUSE,			///< The Pause key

	COUNT,			///< Keep last -- the total number of keyboard keys
	INVALID = -1,	///< Unhandled key
};

enum class eMouseBtn 
{
	LEFT,		///< The left mouse button
	RIGHT,		///< The right mouse button
	MIDDLE,		///< The middle (wheel) mouse button
	EXTRA1,		///< The first extra mouse button
	EXTRA2,		///< The second extra mouse button

	COUNT,
	INVALID = -1
};

enum class eCursorVisual
{
	ARROW,
	IBEAM,
	WAIT,
	CROSS,
	UPARROW,
	SIZE,
	ICON,
	SIZENWSE,
	SIZENESW,
	SIZEWE,
	SIZENS,
	SIZEALL,
	NO,
	HAND,
	APPSTARTING,
	HELP,

	COUNT,
};

class Sys
{
public:
	static DLLHandle	LoadDLL(const wchar_t* path);
	static bool			UnLoadDLL(DLLHandle dllHandle);

	static void			ShowMsgBox(const std::wstring& msg);

	static void			SetCursorPos(const Vec2i& pos);
	static void			SetCursorVisible(bool b);
	static void			SetCursorVisual(eCursorVisual visual, WindowHandle hwnd = nullptr);
	static void*		GetDLLProcAddress(DLLHandle dllHandle, const std::string& procName);

	// TODO REMOVE
	static std::wstring	GetExeDirW();
	static std::string	GetExeDir();

	static Vec2 GetCursorPos();
	static Vec2u GetScreenSize();
};