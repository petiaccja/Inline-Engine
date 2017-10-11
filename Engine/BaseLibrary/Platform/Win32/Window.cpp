#include "Window.hpp"

#include <cassert>
#include <limits>
#include <assert.h>
#include <windowsx.h>
#include <objidl.h>
#include <fstream>

// Drag and drop need OLE initialization this class globally initialize and deinitialize it
class OleManager
{
public:
	OleManager() { OleInitialize(NULL); }
	~OleManager() { OleUninitialize(); }
} OleManager;


// Internal message handler for window
LRESULT CALLBACK WndProc(HWND handle, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Window* window = nullptr;
	if (msg == WM_CREATE)
	{
		window = static_cast<Window*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
		SetWindowLongPtr(handle, -21, reinterpret_cast<LONG_PTR>(window));
	}
	else
	{
		window = reinterpret_cast<Window*>(GetWindowLongPtr(handle, -21));
	}

	if (msg == WM_COPYDATA)
	{
		return TRUE;
	}
	if (msg == WM_SIZE)
	{
		int x = GET_X_LPARAM(lParam);
		int y = GET_Y_LPARAM(lParam);

		RECT clientRect;
		GetClientRect(handle, &clientRect);
		window->onClientSizeChanged(Vec2u(clientRect.right - clientRect.left, clientRect.bottom - clientRect.top));
	}

	auto userWndProc = window ? window->GetUserWndProc() : nullptr;

	LRESULT res;
	if (userWndProc)
		res = window->GetUserWndProc()(handle, msg, wParam, lParam);
	else
		res = DefWindowProc(handle, msg, wParam, lParam);

	return res;
}

Window::Window(const WindowDesc& d)
:userWndProc(nullptr)
{
	dropTarget.Init(this);

	bClosed = false;
	userWndProc = d.userWndProc;

	int interpretedStyle;
	if (d.style == eWindowStyle::BORDERLESS)
	{
		interpretedStyle = WS_POPUP;
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
	wC.lpszClassName = "windowclass";
	wC.lpszMenuName = nullptr;
	wC.hInstance = appID;
	wC.lpfnWndProc = WndProc;
	wC.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClassEx(&wC);

	RECT adjustedsize = { 0 };
	AdjustWindowRect(&adjustedsize, (int)interpretedStyle, 0);

	unsigned width = d.clientSize.x - adjustedsize.left + adjustedsize.right;
	unsigned height = d.clientSize.y - adjustedsize.top + adjustedsize.bottom;

	Vec2u screenSize = Sys::GetScreenSize();
	if(width > screenSize.x)
		width = screenSize.x;

	if(height > screenSize.y)
		height = screenSize.y;

	std::wstring captionText(d.capText.begin(), d.capText.end());

	// Create win32 api window
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
	
	// Show window
	ShowWindow(handle, SW_SHOW);

	// Enable drag & drop
	ChangeWindowMessageFilterEx(handle, WM_DROPFILES, MSGFLT_ALLOW, nullptr);
	ChangeWindowMessageFilterEx(handle, WM_COPYDATA, MSGFLT_ALLOW, nullptr);
	ChangeWindowMessageFilterEx(handle, 0x0049, MSGFLT_ALLOW, nullptr);
	DragAcceptFiles(handle, true);
	HRESULT res = RegisterDragDrop(handle, &dropTarget);
	assert(res == S_OK);

	// Register raw mouse, TODO REMOVE IT
	const unsigned HID_USAGE_PAGE_GENERIC = 0x01;
	const unsigned HID_USAGE_GENERIC_MOUSE = 0x02;
	static bool bInited = false;
	if (!bInited)
	{
		bInited = true;

		RAWINPUTDEVICE Rid[1];
		Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
		Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
		Rid[0].dwFlags = RIDEV_INPUTSINK;
		Rid[0].hwndTarget = handle;
		RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));
	}
}

Window::~Window()
{
	RevokeDragDrop(handle);
}

bool Window::PopEvent(WindowEvent& evt_out)
{
	evt_out.mouseDelta.x = 0;
	evt_out.mouseDelta.y = 0;
	evt_out.key = eKey::INVALID;
	evt_out.mouseBtn = eMouseBtn::INVALID;
	evt_out.msg = eWindowMsg::INVALID;
	evt_out.clientCursorPos.x = -1;
	evt_out.clientCursorPos.y = -1;

	MSG msg;
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	else
	{
		return false;
	}

	if (msg.message == WM_LBUTTONDOWN)
	{
		evt_out.msg = eWindowMsg::MOUSE_PRESS;
		evt_out.mouseBtn = eMouseBtn::LEFT;
		evt_out.clientCursorPos = Vec2(GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam));
		hekk(evt_out.clientCursorPos);
		onMousePressed(evt_out);
	}
	else if (msg.message == WM_RBUTTONDOWN)
	{
		evt_out.msg = eWindowMsg::MOUSE_PRESS;
		evt_out.mouseBtn = eMouseBtn::RIGHT;
		evt_out.clientCursorPos = Vec2(GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam));
		hekk(evt_out.clientCursorPos);
		onMousePressed(evt_out);
	}
	else if (msg.message == WM_MBUTTONDOWN)
	{
		evt_out.msg = eWindowMsg::MOUSE_PRESS;
		evt_out.mouseBtn = eMouseBtn::MIDDLE;
		evt_out.clientCursorPos = Vec2(GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam));
		hekk(evt_out.clientCursorPos);
		onMousePressed(evt_out);
	}
	else if (msg.message == WM_LBUTTONUP)
	{
		evt_out.msg = eWindowMsg::MOUSE_RELEASE;
		evt_out.mouseBtn = eMouseBtn::LEFT;
		evt_out.clientCursorPos = Vec2(GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam));
		hekk(evt_out.clientCursorPos);
		onMouseReleased(evt_out);
	}
	else if (msg.message == WM_RBUTTONUP)
	{
		evt_out.msg = eWindowMsg::MOUSE_RELEASE;
		evt_out.mouseBtn = eMouseBtn::RIGHT;
		evt_out.clientCursorPos = Vec2(GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam));
		hekk(evt_out.clientCursorPos);
		onMouseReleased(evt_out);
	}
	else if (msg.message == WM_MBUTTONUP)
	{
		evt_out.msg = eWindowMsg::MOUSE_RELEASE;
		evt_out.mouseBtn = eMouseBtn::MIDDLE;
		evt_out.clientCursorPos = Vec2(GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam));
		hekk(evt_out.clientCursorPos);
		onMouseReleased(evt_out);
	}
	else if (msg.message == WM_KEYDOWN)
	{
		evt_out.key = ConvertFromWindowsKey(msg.wParam);
		evt_out.msg = eWindowMsg::KEY_PRESS;
	}
	else if (msg.message == WM_SYSKEYDOWN)
	{
		//evt_out.key = ConvertFromWindowsKey(msg.wParam);
		//evt_out.msg = KEY_PRESS;
	}
	else if (msg.message == WM_KEYUP)
	{
		evt_out.key = ConvertFromWindowsKey(msg.wParam);
		evt_out.msg = eWindowMsg::KEY_RELEASE;
	}
	else if (msg.message == WM_SYSKEYUP)
	{
		evt_out.key = ConvertFromWindowsKey(msg.wParam);
		evt_out.msg = eWindowMsg::KEY_RELEASE;
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
			evt_out.mouseDelta = Vec2(raw->data.mouse.lLastX, raw->data.mouse.lLastY);

			POINT p;
			GetCursorPos(&p);
			ScreenToClient(handle, &p);
			evt_out.clientCursorPos = Vec2(p.x, p.y);
			evt_out.msg = eWindowMsg::MOUSE_MOVE;

			hekk(evt_out.clientCursorPos);
			onMouseMoved(evt_out);
		}
	}
	else if (msg.message == WM_CLOSE)
	{
		evt_out.msg = eWindowMsg::CLOSE;
		Close();
	}
	else if (msg.message == WM_QUIT)
	{
		evt_out.msg = eWindowMsg::CLOSE;
		Close();
	}
	else if (msg.message == WM_SIZING)
	{
		int x = GET_X_LPARAM(msg.lParam);
		int y = GET_Y_LPARAM(msg.lParam);
	}
	else
	{
		evt_out.msg = eWindowMsg::INVALID;
	}

	return true;
}

void Window::Close()
{
	RevokeDragDrop(handle);
	CloseWindow(handle);
	bClosed = true;
}

void Window::MinimizeSize()
{
	ShowWindow(handle, SW_MINIMIZE);
}

void Window::MaximizeSize()
{
	ShowWindow(handle, SW_MAXIMIZE);
}

void Window::RestoreSize()
{
	ShowWindow(handle, SW_RESTORE);
}

void Window::SetPos(const Vec2i& pos)
{
	RECT rect;
	GetWindowRect(handle, &rect);
	SetWindowPos(handle, HWND_TOP, pos.x, pos.y, rect.right - rect.left, rect.bottom - rect.top, 0);
}

void Window::SetRect(const Vec2i& pos, const Vec2u& size)
{
	SetWindowPos(handle, HWND_TOP, pos.x, pos.y, size.x, size.y, 0);
}

void Window::SetSize(const Vec2u& size)
{
	RECT rect;
	GetWindowRect(handle, &rect);
	SetWindowPos(handle, HWND_TOP, rect.left, rect.bottom, size.x, size.y, 0);
}

void Window::SetTitle(const std::string& text)
{
	SetWindowTextA(handle, text.c_str());
}

void Window::SetIcon(const std::string& filePath)
{
	HANDLE hIcon = LoadImage(0, filePath.c_str(), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
	if (hIcon)
	{
		//Change both icons to the same icon handle.
		SendMessage((HWND)GetHandle(), WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		SendMessage((HWND)GetHandle(), WM_SETICON, ICON_BIG, (LPARAM)hIcon);

		//This will ensure that the application icon gets changed too.
		SendMessage(GetWindow((HWND)GetHandle(), GW_OWNER), WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		SendMessage(GetWindow((HWND)GetHandle(), GW_OWNER), WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	}
}

bool Window::IsOpen() const
{
	return !bClosed;
}

bool Window::IsFocused() const
{
	return GetFocus() == handle;
}

bool Window::IsMaximizedSize() const
{
	return IsZoomed(handle);
	
}

bool Window::IsMinimizedSize() const
{
	return IsIconic(handle);
}

WindowHandle Window::GetHandle() const
{
	return (WindowHandle)handle;
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

Vec2u Window::GetClientSize() const
{
	RECT rect; GetClientRect(handle, &rect);
	return Vec2u(rect.right - rect.left, rect.bottom - rect.top);
}

Vec2 Window::GetClientCursorPos() const
{
	Vec2 cursorPos = Sys::GetCursorPos();
	POINT p;
	p.x = cursorPos.x;
	p.y = cursorPos.y;
	ScreenToClient(handle, &p);

	return Vec2(p.x, p.y);
}

unsigned Window::GetNumClientPixels() const
{
	return GetClientWidth() * GetClientHeight();
}

float Window::GetClientAspectRatio() const 
{
	return (float)GetClientWidth() * GetClientHeight();
}

Vec2i Window::GetCenterPos() const
{
	WINDOWINFO info;
	assert(GetWindowInfo(handle, &info));

	return Vec2i((int)((info.rcWindow.right - info.rcWindow.left) * 0.5f), (int)((info.rcWindow.bottom - info.rcWindow.right) * 0.5f));
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
	case VK_LCONTROL:	return eKey::LEFT_CONTROL;
	case VK_LSHIFT:		return eKey::LSHIFT;
	case VK_LMENU:		return eKey::LEFT_ALT;
	case VK_LWIN:		return eKey::LSYS;
	case VK_RCONTROL:	return eKey::RIGHT_CONTROL;
	case VK_RSHIFT:		return eKey::RSHIFT;
	case VK_RMENU:		return eKey::RIGHT_ALT;
	case VK_RWIN:		return eKey::RSYS;
	case VK_MENU:		return eKey::LEFT_ALT;
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

	return eKey::INVALID;
}