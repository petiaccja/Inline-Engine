#include "Window.hpp"

#include <cassert>
#include <limits>
#include <assert.h>
#include <windowsx.h>
#include <gdiplus.h>
#include <objidl.h>
#include <fstream>
#pragma comment (lib, "gdiplus.lib")

LRESULT CALLBACK WndProc(HWND handle, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Window* window;
	if (msg == WM_NCCREATE)
	{
		window = static_cast<Window*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
	
		SetLastError(0);
		SetWindowLongPtr(handle, -21, reinterpret_cast<LONG_PTR>(window));
	}
	else
	{
		window = reinterpret_cast<Window*>(GetWindowLongPtr(handle, -21));
	}

	MSG s;
	s.hwnd = handle;
	s.message = msg;
	s.lParam = lParam;
	s.wParam = wParam;

	switch (msg)
	{
	case WM_SIZE:
	case WM_SIZING:
	case WM_DISPLAYCHANGE: window->PostEvent(s); break;
	case WM_PAINT:
	{
		//PAINTSTRUCT ps;
		//BeginPaint(hwnd, &ps);
		if (window->hekkOnPaint)
		{
			window->hekkOnPaint();
		}
		//EndPaint(hwnd, &ps);
		break;
	}
	case WM_DESTROY:
		PostQuitMessage(WM_QUIT);
		break;
	}

	return DefWindowProc(handle, msg, wParam, lParam);
}

Window::Window(const WindowDesc& d)
:hekkOnPaint(nullptr)
{
	bClosed = false;

	int interpretedStyle;
	if (d.style == eWindowStyle::BORDERLESS)
	{
		interpretedStyle = WS_POPUP;
	}
	else if(d.style == eWindowStyle::DEFAULT)
	{
		interpretedStyle = WS_OVERLAPPEDWINDOW;
	}
	else
	{
		interpretedStyle = WS_OVERLAPPEDWINDOW;
	}

	int interpretedBrush = BLACK_BRUSH;

	// Application ID for window class registration, and window creation
	HINSTANCE appID = GetModuleHandle(nullptr);

	// Register our new window class
	WNDCLASSEX wC;
	memset(&wC, 0, sizeof(WNDCLASSEX));
	wC.cbSize = sizeof(WNDCLASSEX);
	wC.cbClsExtra = 0;
	wC.cbWndExtra = 0;
	wC.hbrBackground = NULL;// (HBRUSH)GetStockObject((int)interpretedBrush);
	wC.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wC.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wC.hIconSm = nullptr;
	wC.lpszClassName = L"windowclass";
	wC.lpszMenuName = nullptr;
	wC.hInstance = appID;
	wC.lpfnWndProc = WndProc;
	wC.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClassEx(&wC);

	RECT adjustedsize = { 0 };
	AdjustWindowRect(&adjustedsize, (int)interpretedStyle, 0);

	unsigned width = d.clientSize.x - adjustedsize.left + adjustedsize.right;
	unsigned height = d.clientSize.y - adjustedsize.top + adjustedsize.bottom;

	uvec2 screenSize = Sys::GetScreenSize();
	if(width > screenSize.x)
		width = screenSize.x;

	if(height > screenSize.y)
		height = screenSize.y;

	std::wstring captionText(d.capText.begin(), d.capText.end());

	handle = CreateWindowExW(0,
		L"windowclass",
		captionText.c_str(),
		(int)interpretedStyle,
		0,
		0,
		width,
		height,
		GetDesktopWindow(),
		0,
		appID,
		this);

	ShowWindow(handle, SW_SHOW);
	UpdateWindow(handle);

	//SetWindowLongPtr(hwnd, GWL_USERDATA, (LONG_PTR)this);

	// Register raw mouse
	const unsigned HID_USAGE_PAGE_GENERIC = 0x01;
	const unsigned HID_USAGE_GENERIC_MOUSE = 0x02;

	RAWINPUTDEVICE Rid[1];
	Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
	Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
	Rid[0].dwFlags = RIDEV_INPUTSINK;
	Rid[0].hwndTarget = handle;
	RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));
}

Window::~Window()
{
	Close();
}

void Window::PostEvent(const MSG& msg)
{
	wndProcMessages.push(msg);
}

bool Window::PopEvent(WindowEvent& evt_out)
{
	evt_out.mouseDelta.Zero();
	evt_out.key = INVALID_eKey;
	evt_out.mouseBtn = INVALID_eMouseBtn;
	evt_out.msg = INVALID_eWindowsMsg;
	evt_out.mousePos.Zero();

	MSG msg;
	if (PeekMessage(&msg, handle, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	else // No posted message
	{
		// Still have forwarded messages from wndProc YES !
		if (wndProcMessages.size() > 0)
		{
			msg = wndProcMessages.front();
			wndProcMessages.pop();

			TranslateMessage(&msg);
		}
		else
		{
			return false;
		}
	}

	if (msg.message == WM_LBUTTONDOWN)
	{
		evt_out.msg = MOUSE_PRESS;
		evt_out.mouseBtn = eMouseBtn::LEFT;
		evt_out.mousePos = ivec2(GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam));
		OnMousePress(evt_out);
	}
	else if (msg.message == WM_RBUTTONDOWN)
	{
		evt_out.msg = MOUSE_PRESS;
		evt_out.mouseBtn = eMouseBtn::RIGHT;
		evt_out.mousePos = ivec2(GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam));
		OnMousePress(evt_out);
	}
	else if (msg.message == WM_MBUTTONDOWN)
	{
		evt_out.msg = MOUSE_PRESS;
		evt_out.mouseBtn = eMouseBtn::MID;
		evt_out.mousePos = ivec2(GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam));
		OnMousePress(evt_out);
	}
	else if (msg.message == WM_LBUTTONUP)
	{
		evt_out.msg = MOUSE_RELEASE;
		evt_out.mouseBtn = eMouseBtn::LEFT;
		evt_out.mousePos = ivec2(GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam));
		OnMouseRelease(evt_out);
	}
	else if (msg.message == WM_RBUTTONUP)
	{
		evt_out.msg = MOUSE_RELEASE;
		evt_out.mouseBtn = eMouseBtn::RIGHT;
		evt_out.mousePos = ivec2(GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam));
		OnMouseRelease(evt_out);
	}
	else if (msg.message == WM_MBUTTONUP)
	{
		evt_out.msg = MOUSE_RELEASE;
		evt_out.mouseBtn = eMouseBtn::MID;
		evt_out.mousePos = ivec2(GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam));
		OnMouseRelease(evt_out);
	}
	else if (msg.message == WM_KEYDOWN)
	{
		evt_out.key = ConvertFromWindowsKey(msg.wParam);
		evt_out.msg = KEY_PRESS;
	}
	else if (msg.message == WM_SYSKEYDOWN)
	{
		//evt_out.key = ConvertFromWindowsKey(msg.wParam);
		//evt_out.msg = KEY_PRESS;
	}
	else if (msg.message == WM_KEYUP)
	{
		evt_out.key = ConvertFromWindowsKey(msg.wParam);
		evt_out.msg = KEY_RELEASE;
	}
	else if (msg.message == WM_SYSKEYUP)
	{
		evt_out.key = ConvertFromWindowsKey(msg.wParam);
		evt_out.msg = KEY_RELEASE;
	}
	else if(msg.message == WM_INPUT)
	{
		UINT dwSize;
		GetRawInputData((HRAWINPUT)msg.lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));

		BYTE* data = (BYTE*)alloca(dwSize);

		GetRawInputData((HRAWINPUT)msg.lParam, RID_INPUT, data, &dwSize, sizeof(RAWINPUTHEADER));
		

		RAWINPUT* raw = (RAWINPUT*)data;

		if (raw->header.dwType == RIM_TYPEMOUSE)
		{
			evt_out.mouseDelta = ivec2(raw->data.mouse.lLastX, raw->data.mouse.lLastY);
			evt_out.msg = MOUSE_MOVE;
		}
	}
	else if (msg.message == WM_CLOSE)
	{
		evt_out.msg = CLOSE;
		Close();
	}
	else if (msg.message == WM_QUIT)
	{
		evt_out.msg = CLOSE;
		Close();
	}
	else if (msg.message == WM_DISPLAYCHANGE)
	{
		// TODO
	}
	else if (msg.message == WM_SIZE)
	{
		// TODO
		/*std::ofstream os("WM_SIZE.txt", std::ios::app);
		os << GET_X_LPARAM(msg.lParam);
		os << '\n';
		os << GET_Y_LPARAM(msg.lParam);
		os << '\n';
		os.close();*/
	}
	else if (msg.message == WM_SIZING)
	{

	}
	else
	{
		evt_out.msg = INVALID_eWindowsMsg;
	}

	return true;
}

void Window::Close() 
{
	CloseWindow(handle);
	bClosed = true;
}

void Window::Clear(const Color& color)
{
	// TODO
}

void Window::SetPos(const ivec2& pos /*= ivec2(0, 0)*/)
{
	RECT rect;
	GetWindowRect(handle, &rect);
	SetWindowPos(handle, HWND_TOP, pos.x, pos.y, rect.right - rect.left, rect.bottom - rect.top, 0);
}

void Window::SetSize(const uvec2& size)
{
	RECT rect;
	GetWindowRect(handle, &rect);
	SetWindowPos(handle, HWND_TOP, rect.left, rect.bottom, size.x, size.y, 0);
}

void Window::SetClientPixels(const Color* const pixels)
{
	// TODO
}

void Window::SetCursorVisible(bool bVisible)
{
	// TODO
}

void Window::SetTitle(const std::wstring& text)
{
	SetWindowText(handle, text.c_str());
}

bool Window::IsOpen() const
{
	return !bClosed;
}

bool Window::IsFocused() const
{
	return GetFocus() == handle;
}

size_t Window::GetHandle() const
{
	return (size_t)handle;
}

uint32_t Window::GetClientWidth() const 
{
	RECT rect; GetClientRect(handle, &rect);
	return (uint32_t)(rect.right - rect.left);
}

uint32_t Window::GetClientHeight() const
{
	RECT rect; GetClientRect(handle, &rect);
	return (uint32_t)(rect.bottom - rect.top);
}

ivec2 Window::GetClientCursorPos() const
{
	ivec2 cursorPos = Sys::GetCursorPos();
	POINT p;
	p.x = cursorPos.x;
	p.y = cursorPos.y;
	ScreenToClient(handle, &p);

	return ivec2(p.x, p.y);
}

unsigned Window::GetNumClientPixels() const
{
	return GetClientWidth() * GetClientHeight();
}

float Window::GetClientAspectRatio() const 
{
	return (float)GetClientWidth() * GetClientHeight();
}

ivec2 Window::GetCenterPos() const
{
	WINDOWINFO info;
	assert(GetWindowInfo(handle, &info));

	return ivec2((int)((info.rcWindow.right - info.rcWindow.left) * 0.5f), (int)((info.rcWindow.bottom - info.rcWindow.right) * 0.5f));
}

eKey Window::ConvertFromWindowsKey(WPARAM key)
{
	switch (key)
	{
	case 0x41:			return eKey::A;
	case 0x42:			return eKey::B;
	case 0x43:			return eKey::C;
	case 0x44:			return eKey::D;
	case 0x45:			return eKey::E;
	case 0x46:			return eKey::F;
	case 0x47:			return eKey::G;
	case 0x48:			return eKey::H;
	case 0x49:			return eKey::I;
	case 0x4A:			return eKey::J;
	case 0x4B:			return eKey::K;
	case 0x4C:			return eKey::L;
	case 0x4D:			return eKey::M;
	case 0x4E:			return eKey::N;
	case 0x4F:			return eKey::O;
	case 0x50:			return eKey::P;
	case 0x51:			return eKey::Q;
	case 0x52:			return eKey::R;
	case 0x53:			return eKey::S;
	case 0x54:			return eKey::T;
	case 0x55:			return eKey::U;
	case 0x56:			return eKey::V;
	case 0x57:			return eKey::W;
	case 0x58:			return eKey::X;
	case 0x59:			return eKey::Y;
	case 0x5A:			return eKey::Z;
	case VK_NUMPAD0:	return eKey::NUM0;
	case VK_NUMPAD1:	return eKey::NUM1;
	case VK_NUMPAD2:	return eKey::NUM2;
	case VK_NUMPAD3:	return eKey::NUM3;
	case VK_NUMPAD4:	return eKey::NUM4;
	case VK_NUMPAD5:	return eKey::NUM5;
	case VK_NUMPAD6:	return eKey::NUM6;
	case VK_NUMPAD7:	return eKey::NUM7;
	case VK_NUMPAD8:	return eKey::NUM8;
	case VK_NUMPAD9:	return eKey::NUM9;
	case VK_ESCAPE:		return eKey::ESC;
	case VK_LCONTROL:	return eKey::LCTRL;
	case VK_LSHIFT:		return eKey::LSHIFT;
	case VK_LMENU:		return eKey::LALT;
	case VK_LWIN:		return eKey::LSYS;
	case VK_RCONTROL:	return eKey::RCTRL;
	case VK_RSHIFT:		return eKey::RSHIFT;
	case VK_RMENU:		return eKey::RALT;
	case VK_RWIN:		return eKey::RSYS;
	case VK_MENU:		return eKey::LALT;
	// TODO
	//case sf::Keyboard::LBracket:	return eKey::LBRACKET;
	//case sf::Keyboard::RBracket:	return eKey::RBRACKET;
	//case sf::Keyboard::SemiColon:	return eKey::SEMICOLON;
	//case sf::Keyboard::Comma:		return eKey::COMMA;
	//case sf::Keyboard::Period:		return eKey::PERIOD;
	//case sf::Keyboard::Quote:		return eKey::QUOTE;
	//case sf::Keyboard::Slash:		return eKey::SLASH;
	//case sf::Keyboard::BackSlash:	return eKey::BACKSLASH;
	//case sf::Keyboard::Tilde:		return eKey::TILDE;
	//case sf::Keyboard::Equal:		return eKey::EQUAL;
	//case sf::Keyboard::Dash:		return eKey::DASH;
	case VK_SPACE:					return eKey::SPACE;
	case VK_RETURN:					return eKey::ENTER;
	case VK_BACK:					return eKey::BACKSPACE;
	case VK_TAB:					return eKey::TAB;
	//case sf::Keyboard::PageUp:		return eKey::PAGEUP;
	//case sf::Keyboard::PageDown:	return eKey::PAGEDDOWN;
	case VK_END:					return eKey::END;
	case VK_HOME:					return eKey::HOME;
	case VK_INSERT:					return eKey::INS;
	case VK_DELETE:					return eKey::DEL;
	case VK_ADD:					return eKey::ADD;
	case VK_SUBTRACT:				return eKey::SUB;
	case VK_MULTIPLY:				return eKey::MUL;
	case VK_DIVIDE:					return eKey::DIV;
	case VK_LEFT:					return eKey::LEFT_ARROW;
	case VK_RIGHT:					return eKey::RIGHT_ARROW;
	case VK_UP:						return eKey::UP;
	case VK_DOWN:					return eKey::DOWN;
	case VK_F1:						return eKey::F1;
	case VK_F2:						return eKey::F2;
	case VK_F3:						return eKey::F3;
	case VK_F4:						return eKey::F4;
	case VK_F5:						return eKey::F5;
	case VK_F6:						return eKey::F6;
	case VK_F7:						return eKey::F7;
	case VK_F8:						return eKey::F8;
	case VK_F9:						return eKey::F9;
	case VK_F10:					return eKey::F10;
	case VK_F11:					return eKey::F11;
	case VK_F12:					return eKey::F12;
	case VK_F13:					return eKey::F13;
	case VK_F14:					return eKey::F14;
	case VK_F15:					return eKey::F15;
	case VK_F16:					return eKey::F16;
	case VK_F17:					return eKey::F17;
	case VK_F18:					return eKey::F18;
	case VK_F19:					return eKey::F19;
	case VK_F20:					return eKey::F20;
	case VK_F21:					return eKey::F21;
	case VK_F22:					return eKey::F22;
	case VK_F23:					return eKey::F23;
	case VK_F24:					return eKey::F24;
	case VK_PAUSE:					return eKey::PAUSE;
	}

	return INVALID_eKey;
}