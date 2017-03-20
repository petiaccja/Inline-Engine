#include "Window.hpp"

#include <cassert>
#include <limits>
#include <assert.h>
#include <windowsx.h>
#include <gdiplus.h>
#include <objidl.h>
#include <fstream>
#pragma comment (lib, "gdiplus.lib")

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Window* window;
	if (msg == WM_NCCREATE)
	{
		window = static_cast<Window*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
	
		SetLastError(0);
		SetWindowLongPtr(hwnd, -21, reinterpret_cast<LONG_PTR>(window));
	}
	else
	{
		window = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, -21));
	}

	MSG s;
	s.hwnd = hwnd;
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

	return DefWindowProc(hwnd, msg, wParam, lParam);
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

	hwnd = CreateWindowExW(0,
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

	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	//SetWindowLongPtr(hwnd, GWL_USERDATA, (LONG_PTR)this);

	// Register raw mouse
	const unsigned HID_USAGE_PAGE_GENERIC = 0x01;
	const unsigned HID_USAGE_GENERIC_MOUSE = 0x02;

	RAWINPUTDEVICE Rid[1];
	Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
	Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
	Rid[0].dwFlags = RIDEV_INPUTSINK;
	Rid[0].hwndTarget = hwnd;
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
	if (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE))
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
	CloseWindow(hwnd);
	bClosed = true;
}

void Window::Clear(const Color& color)
{
	// TODO
	//w.clear(sf::Color(color.r, color.g, color.b, color.a));
}

void Window::SetPos(const ivec2& pos /*= ivec2(0, 0)*/)
{
	RECT rect;
	GetWindowRect(hwnd, &rect);
	SetWindowPos(hwnd, HWND_TOP, pos.x, pos.y, rect.right - rect.left, rect.bottom - rect.top, 0);
	//w.setPosition(sf::Vector2i(pos.x, pos.y));
}

void Window::SetSize(const uvec2& size)
{
	RECT rect;
	GetWindowRect(hwnd, &rect);
	SetWindowPos(hwnd, HWND_TOP, rect.left, rect.bottom, size.x, size.y, 0);
	//w.setSize(sf::Vector2u(pos.x, pos.y));
}

void Window::SetClientPixels(const Color* const pixels)
{
	// TODO
	//static thread_local sf::Sprite	sprite;
	//static thread_local sf::Texture texture;
	//static thread_local bool bInited = false;
	//
	//if (!bInited)
	//{
	//	texture.create(GetClientWidth(), GetClientHeight());
	//	sprite.setTexture(texture);
	//	bInited = true;
	//}
	//
	//texture.update((uint8_t*)pixels);
	//
	//w.draw(sprite);
}

void Window::SetCursorVisible(bool bVisible)
{
	// TODO
	//w.setMouseCursorVisible(bVisible);
}

void Window::SetTitle(const std::wstring& text)
{
	SetWindowText(hwnd, text.c_str());
}

bool Window::IsOpen() const
{
	return !bClosed;
}

size_t Window::GetHandle() const
{
	return (size_t)hwnd;
}

uint32_t Window::GetClientWidth() const 
{
	RECT rect; GetClientRect(hwnd, &rect);
	return (uint32_t)(rect.right - rect.left);
}

uint32_t Window::GetClientHeight() const
{
	RECT rect; GetClientRect(hwnd, &rect);
	return (uint32_t)(rect.bottom - rect.top);
}

ivec2 Window::GetClientCursorPos() const
{
	ivec2 cursorPos = Sys::GetCursorPos();
	POINT p;
	p.x = cursorPos.x;
	p.y = cursorPos.y;
	ScreenToClient(hwnd, &p);

	return ivec2(p.x, p.y);
}

unsigned Window::GetNumClientPixels() const
{
	return GetClientWidth() * GetClientHeight();
}

float Window::GetClientAspectRatio() const 
{
	return (float)GetClientWidth() * GetClientHeight();
	//const sf::Vector2u size = w.getSize();
	//return (float)size.x / size.y;
}

ivec2 Window::GetCenterPos() const
{
	WINDOWINFO info;
	assert(GetWindowInfo(hwnd, &info));

	return ivec2((int)((info.rcWindow.right - info.rcWindow.left) * 0.5f), (int)((info.rcWindow.bottom - info.rcWindow.right) * 0.5f));
	//auto size = w.getSize();
	//auto pos = w.getPosition();
	//
	//return ivec2(pos.x + size.x * 0.5, pos.y + size.y * 0.5);
}

//eWindowMsg Window::ConvertFromSFMLWindowMsg(sf::Event::EventType windowMsg)
//{
//	switch (windowMsg)
//	{
//	case sf::Event::EventType::Closed:					return CLOSE;
//	case sf::Event::EventType::Resized:					return RESIZE;
//	case sf::Event::EventType::LostFocus:				return DEFOCUS;
//	case sf::Event::EventType::GainedFocus:				return FOCUS;
//	case sf::Event::EventType::TextEntered:				return TEXT_ENTERED;
//	case sf::Event::EventType::KeyPressed:				return KEY_PRESS;
//	case sf::Event::EventType::KeyReleased:				return KEY_RELEASE;
//	case sf::Event::EventType::MouseWheelScrolled:
//	case sf::Event::EventType::MouseWheelMoved:			return MOUSE_SCROLL;
//	case sf::Event::EventType::MouseButtonPressed:		return MOUSE_PRESS;
//	case sf::Event::EventType::MouseButtonReleased:		return MOUSE_RELEASE;
//	case sf::Event::EventType::MouseMoved:				return MOUSE_MOVE;
//	case sf::Event::EventType::MouseEntered:			return MOUSE_ENTER;
//	case sf::Event::EventType::MouseLeft:				return MOUSE_LEAVE;
//	case sf::Event::EventType::JoystickButtonPressed:	return JOYSTICK_BUTTON_PRESS;
//	case sf::Event::EventType::JoystickButtonReleased:	return JOYSTICK_BUTTON_RELEASE;
//	case sf::Event::EventType::JoystickMoved:			return JOYSTICK_MOVE;
//	case sf::Event::EventType::JoystickConnected:		return JOYSTICK_CONNECT;
//	case sf::Event::EventType::JoystickDisconnected:	return JOYSTICK_DISCONNECT;
//	}
//
//	return INVALID;
//}
//
//eMouseBtn Window::ConvertFromSFMLMouseBtn(sf::Mouse::Button btn)
//{
//	switch (btn)
//	{
//		case sf::Mouse::Button::Left:		return eMouseBtn::LEFT;
//		case sf::Mouse::Button::Right:		return eMouseBtn::RIGHT;
//		case sf::Mouse::Button::Middle:		return eMouseBtn::MID;
//		case sf::Mouse::Button::XButton1:	return eMouseBtn::EXTRA1;
//		case sf::Mouse::Button::XButton2:	return eMouseBtn::EXTRA2;
//	}
//
//	return eMouseBtn::INVALID;
//}

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

//eKey Window::ConvertFromSFMLKey(sf::Keyboard::Key key)
//{
//	switch (key)
//	{
//	case sf::Keyboard::A:			return eKey::A;
//	case sf::Keyboard::B:			return eKey::B;
//	case sf::Keyboard::C:			return eKey::C;
//	case sf::Keyboard::D:			return eKey::D;
//	case sf::Keyboard::E:			return eKey::E;
//	case sf::Keyboard::F:			return eKey::F;
//	case sf::Keyboard::G:			return eKey::G;
//	case sf::Keyboard::H:			return eKey::H;
//	case sf::Keyboard::I:			return eKey::I;
//	case sf::Keyboard::J:			return eKey::J;
//	case sf::Keyboard::K:			return eKey::K;
//	case sf::Keyboard::L:			return eKey::L;
//	case sf::Keyboard::M:			return eKey::M;
//	case sf::Keyboard::N:			return eKey::N;
//	case sf::Keyboard::O:			return eKey::O;
//	case sf::Keyboard::P:			return eKey::P;
//	case sf::Keyboard::Q:			return eKey::Q;
//	case sf::Keyboard::R:			return eKey::R;
//	case sf::Keyboard::S:			return eKey::S;
//	case sf::Keyboard::T:			return eKey::T;
//	case sf::Keyboard::U:			return eKey::U;
//	case sf::Keyboard::V:			return eKey::V;
//	case sf::Keyboard::W:			return eKey::W;
//	case sf::Keyboard::X:			return eKey::X;
//	case sf::Keyboard::Y:			return eKey::Y;
//	case sf::Keyboard::Z:			return eKey::Z;
//	case sf::Keyboard::Num0:		return eKey::NUM0;
//	case sf::Keyboard::Num1:		return eKey::NUM1;
//	case sf::Keyboard::Num2:		return eKey::NUM2;
//	case sf::Keyboard::Num3:		return eKey::NUM3;
//	case sf::Keyboard::Num4:		return eKey::NUM4;
//	case sf::Keyboard::Num5:		return eKey::NUM5;
//	case sf::Keyboard::Num6:		return eKey::NUM6;
//	case sf::Keyboard::Num7:		return eKey::NUM7;
//	case sf::Keyboard::Num8:		return eKey::NUM8;
//	case sf::Keyboard::Num9:		return eKey::NUM9;
//	case sf::Keyboard::Escape:		return eKey::ESC;
//	case sf::Keyboard::LControl:	return eKey::LCTRL;
//	case sf::Keyboard::LShift:		return eKey::LSHIFT;
//	case sf::Keyboard::LAlt:		return eKey::LALT;
//	case sf::Keyboard::LSystem:		return eKey::LSYS;
//	case sf::Keyboard::RControl:	return eKey::RCTRL;
//	case sf::Keyboard::RShift:		return eKey::RSHIFT;
//	case sf::Keyboard::RAlt:		return eKey::RALT;
//	case sf::Keyboard::RSystem:		return eKey::RSYS;
//	case sf::Keyboard::Menu:		return eKey::MENU;
//	case sf::Keyboard::LBracket:	return eKey::LBRACKET;
//	case sf::Keyboard::RBracket:	return eKey::RBRACKET;
//	case sf::Keyboard::SemiColon:	return eKey::SEMICOLON;
//	case sf::Keyboard::Comma:		return eKey::COMMA;
//	case sf::Keyboard::Period:		return eKey::PERIOD;
//	case sf::Keyboard::Quote:		return eKey::QUOTE;
//	case sf::Keyboard::Slash:		return eKey::SLASH;
//	case sf::Keyboard::BackSlash:	return eKey::BACKSLASH;
//	case sf::Keyboard::Tilde:		return eKey::TILDE;
//	case sf::Keyboard::Equal:		return eKey::EQUAL;
//	case sf::Keyboard::Dash:		return eKey::DASH;
//	case sf::Keyboard::Space:		return eKey::SPACE;
//	case sf::Keyboard::Return:		return eKey::RETURN;
//	case sf::Keyboard::BackSpace:	return eKey::BACKSPACE;
//	case sf::Keyboard::Tab:			return eKey::TAB;
//	case sf::Keyboard::PageUp:		return eKey::PAGEUP;
//	case sf::Keyboard::PageDown:	return eKey::PAGEDDOWN;
//	case sf::Keyboard::End:			return eKey::END;
//	case sf::Keyboard::Home:		return eKey::HOME;
//	case sf::Keyboard::Insert:		return eKey::INS;
//	case sf::Keyboard::Delete:		return eKey::DEL;
//	case sf::Keyboard::Add:			return eKey::ADD;
//	case sf::Keyboard::Subtract:	return eKey::SUB;
//	case sf::Keyboard::Multiply:	return eKey::MUL;
//	case sf::Keyboard::Divide:		return eKey::DIV;
//	case sf::Keyboard::Left:		return eKey::LEFT;
//	case sf::Keyboard::Right:		return eKey::RIGHT;
//	case sf::Keyboard::Up:			return eKey::UP;
//	case sf::Keyboard::Down:		return eKey::DOWN;
//	case sf::Keyboard::Numpad0:		return eKey::NUM0;
//	case sf::Keyboard::Numpad1:		return eKey::NUM1;
//	case sf::Keyboard::Numpad2:		return eKey::NUM2;
//	case sf::Keyboard::Numpad3:		return eKey::NUM3;
//	case sf::Keyboard::Numpad4:		return eKey::NUM4;
//	case sf::Keyboard::Numpad5:		return eKey::NUM5;
//	case sf::Keyboard::Numpad6:		return eKey::NUM6;
//	case sf::Keyboard::Numpad7:		return eKey::NUM7;
//	case sf::Keyboard::Numpad8:		return eKey::NUM8;
//	case sf::Keyboard::Numpad9:		return eKey::NUM9;
//	case sf::Keyboard::F1:			return eKey::F1;
//	case sf::Keyboard::F2:			return eKey::F2;
//	case sf::Keyboard::F3:			return eKey::F3;
//	case sf::Keyboard::F4:			return eKey::F4;
//	case sf::Keyboard::F5:			return eKey::F5;
//	case sf::Keyboard::F6:			return eKey::F6;
//	case sf::Keyboard::F7:			return eKey::F7;
//	case sf::Keyboard::F8:			return eKey::F8;
//	case sf::Keyboard::F9:			return eKey::F9;
//	case sf::Keyboard::F10:			return eKey::F10;
//	case sf::Keyboard::F11:			return eKey::F11;
//	case sf::Keyboard::F12:			return eKey::F12;
//	case sf::Keyboard::F13:			return eKey::F13;
//	case sf::Keyboard::F14:			return eKey::F14;
//	case sf::Keyboard::F15:			return eKey::F15;
//	case sf::Keyboard::Pause:		return eKey::PAUSE;
//	case sf::Keyboard::KeyCount:	return eKey::COUNT;
//	}
//
//	return eKey::INVALID;
//}
//
//sf::Uint32	Window::ConvertToSFMLWindowStyle(eWindowStyle style)
//{
//	switch (style)
//	{
//	case eWindowStyle::DEFAULT:							return sf::Style::Default;
//	case eWindowStyle::BORDERLESS:						return sf::Style::None;
//	case eWindowStyle::TITLE_FIXBORDER:					return sf::Style::Titlebar;
//	case eWindowStyle::TITLE_RESIZEABLEBORDER:			return sf::Style::Titlebar | sf::Style::Resize;
//	case eWindowStyle::TITLE_CLOSEABLE:					return sf::Style::Titlebar | sf::Style::Close;
//	}
//
//	return sf::Style::Default;
//}