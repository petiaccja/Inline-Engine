// Interface for typical O.S Window
#pragma once

#include "Sys.hpp"

#include <string>
#include <functional>
#include <vector>
#include <filesystem>

using namespace std::experimental::filesystem;

ENUM_CLASS_BITFLAG( eWindowStyle, int )
{
	DEFAULT = 1,
	BORDERLESS = 2,
	TITLE_FIXBORDER = 4,
	TITLE_RESIZEABLEBORDER = 8,
	TITLE_CLOSEABLE = 16
};

enum eWindowMsg 
{
	CLOSE,						///< The window requested to be closed (no data)
	RESIZE,						///< The window was resized (data in event.size)
	DEFOCUS,					///< The window lost the focus (no data)
	FOCUS,						///< The window gained the focus (no data)
	TEXT_ENTERED,				///< A character was entered (data in event.text)
	KEY_PRESS,					///< A key was pressed (data in event.key)
	KEY_RELEASE,				///< A key was released (data in event.key)
	MOUSE_SCROLL,				///< The mouse wheel was scrolled (data in event.mouseWheel)
	MOUSE_PRESS,				///< A mouse button was pressed (data in event.mouseButton)
	MOUSE_RELEASE,				///< A mouse button was released (data in event.mouseButton)
	MOUSE_MOVE,					///< The mouse cursor moved (data in event.mouseMove)
	MOUSE_ENTER,				///< The mouse cursor entered the area of the window (no data)
	MOUSE_LEAVE,				///< The mouse cursor left the area of the window (no data)
	JOYSTICK_BUTTON_PRESS,		///< A joystick button was pressed (data in event.joystickButton)
	JOYSTICK_BUTTON_RELEASE,	///< A joystick button was released (data in event.joystickButton)
	JOYSTICK_MOVE,				///< The joystick moved along an axis (data in event.joystickMove)
	JOYSTICK_CONNECT,			///< A joystick was connected (data in event.joystickConnect)
	JOYSTICK_DISCONNECT,		///< A joystick was disconnected (data in event.joystickConnect)
	COUNT_eWindowsMsg,			///< Keep last -- the total number of event types
	INVALID_eWindowsMsg = -1,
};

struct WindowEvent
{
	WindowEvent(): msg(INVALID_eWindowsMsg), key(INVALID_eKey), mouseDelta(0, 0), clientMousePos(0, 0) {}

	eWindowMsg	msg;
	eKey		key;
	eMouseBtn	mouseBtn;
	Vec2	mouseDelta;
	Vec2	clientMousePos;
};

struct WindowDesc
{
	WindowDesc() : style(eWindowStyle::DEFAULT), clientSize(0, 0) {}

	std::string	 capText;
	eWindowStyle style;
	Vec2u		 clientSize;
	std::function<LRESULT(WindowHandle, unsigned int, long long, long long)> userWndProc;
};

struct DragData
{
	std::wstring text;
	std::vector<path> filesPaths;

	void Reset()
	{
		text = L"";
		filesPaths.clear();
	}
};