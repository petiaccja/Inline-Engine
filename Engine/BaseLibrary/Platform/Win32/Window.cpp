#include "Window.hpp"
#include "../../Exception/Exception.hpp"
#include <future>
#include <Windowsx.h>
#include "shellapi.h"
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#undef UNKNOWN

#undef IsMaximized
#undef IsMinimized


namespace inl {



Window::Window(const std::string& title,
	Vec2u size,
	bool borderless,
	bool resizable,
	bool hiddenInitially)
{
	// Lazy-register window class.
	static bool isWcRegistered = [] {
		WNDCLASSEXA wc;
		wc.cbClsExtra = 0;
		wc.cbSize = sizeof(wc);
		wc.cbWndExtra = 0;
		wc.hCursor = NULL;
		wc.hIcon = NULL;
		wc.hIconSm = NULL;
		wc.hInstance = GetModuleHandle(NULL);
		wc.hbrBackground = NULL;
		wc.lpfnWndProc = &Window::WndProc;
		wc.lpszClassName = "INL_SIMPLE_WINDOW_CLASS";
		wc.lpszMenuName = nullptr;
		wc.style = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;

		ATOM cres = RegisterClassExA(&wc);
		if (cres == 0) {
			DWORD error = GetLastError();
			throw RuntimeException("Failed to register inline engine window class.", std::to_string(error));
		}
		return true;
	}();


	if (!isWcRegistered) {
		throw RuntimeException("Window class is not registered.");
	}


	// Create window on current thread
	HWND hwnd = NULL;
	try {
		// Create the WINAPI window itself.
		hwnd = CreateWindowExA(
			0,
			"INL_SIMPLE_WINDOW_CLASS",
			title.c_str(),
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			size.x,
			size.y,
			NULL,
			NULL,
			GetModuleHandle(NULL),
			(void*)this);

		if (hwnd == NULL) {
			DWORD error = GetLastError();
			throw RuntimeException("Failed to create window.", std::to_string(error));
		}

		// Init OLE and drag and drop for this thread.
		HRESULT res;
		res = OleInitialize(nullptr);
		if (res != S_OK) {
			throw RuntimeException("Could not initialize OLE on thread.");
		}
		res = RegisterDragDrop(hwnd, this);
		if (res != S_OK) {
			throw RuntimeException("Failed to set drag'n'drop for window.");
		}
	}
	catch (...) {
		if (hwnd) {
			DestroyWindow(hwnd);
		}
		throw;
	}

	// Show and update the newly created window.
	m_handle = hwnd;
	if (!hiddenInitially) {
		ShowWindow(m_handle, SW_SHOW);
	}
	UpdateWindow(m_handle);


	// Handle borderless and resize properties.
	SetBorderless(borderless);
	SetResizable(resizable);
	SetTitle(title);
}


Window::Window(Window&& rhs) noexcept {
	m_handle = rhs.m_handle;

	rhs.m_handle = NULL;
}


Window& Window::operator=(Window&& rhs) noexcept {
	m_handle = rhs.m_handle;

	rhs.m_handle = NULL;

	return *this;
}


Window::~Window() {
	if (m_handle != 0) {
		DestroyWindow(m_handle);
	}
	if (m_icon) {
		DestroyIcon((HICON)m_icon);
	}
}



bool Window::IsClosed() const {
	return m_handle == NULL;
}

void Window::Close() {
	CloseWindow(m_handle);
	m_handle = nullptr;
}

void Window::Show() {
	if (IsClosed()) {
		return;
	}
	ShowWindow(m_handle, SW_SHOW);
	UpdateWindow(m_handle);
}


void Window::Hide() {
	if (IsClosed()) {
		return;
	}
	ShowWindow(m_handle, SW_HIDE);
}


bool Window::IsFocused() const {
	return GetFocus() == m_handle;
}


WindowHandle Window::GetNativeHandle() const {
	return m_handle;
}


void Window::Maximize() {
	if (IsClosed()) {
		return;
	}
	ShowWindow(m_handle, SW_MAXIMIZE);
}


void Window::Minize() {
	if (IsClosed()) {
		return;
	}
	ShowWindow(m_handle, SW_MINIMIZE);
}


void Window::Restore() {
	if (IsClosed()) { return; }
	ShowWindow(m_handle, SW_RESTORE);
}


bool Window::IsMaximized() const {
	if (IsClosed()) { return false; }
	return IsZoomed(m_handle);
}
bool Window::IsMinimized() const {
	if (IsClosed()) { return false; }
	return IsIconic(m_handle);
}

void Window::SetSize(const Vec2u& size) {
	if (IsClosed()) { return; }
	SetWindowPos(m_handle, NULL, 0, 0, size.x, size.y, SWP_NOMOVE);
}


Vec2u Window::GetSize() const {
	if (IsClosed()) { return { 0,0 }; }
	RECT rc;
	GetWindowRect(m_handle, &rc);
	return { rc.right - rc.left, rc.bottom - rc.top };
}


Vec2u Window::GetClientSize() const {
	if (IsClosed()) { return { 0,0 }; }
	RECT rc;
	GetClientRect(m_handle, &rc);
	return { rc.right - rc.left, rc.bottom - rc.top };
}


void Window::SetPosition(const Vec2i& position) {
	if (IsClosed()) { return; }
	SetWindowPos(m_handle, NULL, position.x, position.y, 0, 0, SWP_NOMOVE);
}


Vec2i Window::GetPosition() const {
	if (IsClosed()) { return { 0,0 }; }
	RECT rc;
	GetWindowRect(m_handle, &rc);
	return { rc.left, rc.top };
}

Vec2i Window::GetClientCursorPos() const
{
	POINT p;
	GetCursorPos(&p);
	ScreenToClient(m_handle, &p);
	return Vec2i(p.x, p.y);
}



void Window::SetFrameMargins(RectI frameMargins) {
	m_frameMargins = frameMargins;
}


RectI Window::GetFrameMargins() const {
	return m_frameMargins;
}


void Window::SetCaptionButtonHandler(std::function<eWindowCaptionButton(Vec2i)> handler) {
	m_captionButtonHandler = handler;
}


std::function<eWindowCaptionButton(Vec2i)> Window::GetCaptionButtonHandler() const {
	return m_captionButtonHandler;
}


void Window::SetResizable(bool enabled) {
	if (IsClosed()) { return; }
	m_resizable = enabled;
	if (enabled) {
		SetWindowLong(m_handle, GWL_STYLE, GetWindowLong(m_handle, GWL_STYLE)|WS_SIZEBOX);
	}
	else {
		SetWindowLong(m_handle, GWL_STYLE, GetWindowLong(m_handle, GWL_STYLE)&~WS_SIZEBOX);
	}
}


bool Window::GetResizable() const {
	if (IsClosed()) { return false; }
	return m_resizable;
}


void Window::SetBorderless(bool enabled) {
	if (IsClosed()) { return; }
	m_borderless = enabled;

	// Must call SetWindowPos to trigger changes
	RECT rc;
	GetWindowRect(m_handle, &rc);
	SetWindowPos(m_handle,
		NULL,
		rc.left, rc.top,
		rc.right-rc.left, rc.bottom-rc.top,
		SWP_FRAMECHANGED);
}


int Window::DwmHittest(Vec2i cursorPos) const {
	Vec2u windowSize = GetSize();

	bool onLeftBorder = cursorPos.x < m_frameMargins.left;
	bool onRightBorder = cursorPos.x > (int)windowSize.x - m_frameMargins.right;
	bool onTopBorder = cursorPos.y < m_frameMargins.top;
	bool onBottomBorder = cursorPos.y > (int)windowSize.y - m_frameMargins.bottom;

	if (onTopBorder && onLeftBorder) {
		return HTTOPLEFT;
	}
	if (onTopBorder && onRightBorder) {
		return HTTOPRIGHT;
	}
	if (onBottomBorder && onRightBorder) {
		return HTBOTTOMRIGHT;
	}
	if (onBottomBorder && onLeftBorder) {
		return HTBOTTOMLEFT;
	}
	if (onLeftBorder) {
		return HTLEFT;
	}
	if (onRightBorder) {
		return HTRIGHT;
	}
	if (onTopBorder) {
		return HTTOP;
	}
	if (onBottomBorder) {
		return HTBOTTOM;
	}

	eWindowCaptionButton onCaption = eWindowCaptionButton::NONE;
	if (m_captionButtonHandler) {
		onCaption = m_captionButtonHandler(cursorPos);
	}
	switch (onCaption) {
		case eWindowCaptionButton::BAR:
			return HTCAPTION;
		case eWindowCaptionButton::MINIMIZE:
			return HTMINBUTTON;
		case eWindowCaptionButton::MAXIMIZE:
			return HTMAXBUTTON;
		case eWindowCaptionButton::CLOSE:
			return HTCLOSE;
	}

	return HTCLIENT;
}


bool Window::GetBorderless() const {
	if (IsClosed()) { return false; }
	return m_borderless;
}


void Window::SetTitle(const std::string& text) {
	if (IsClosed()) { return; }
	SetWindowTextA(m_handle, text.c_str());
}


std::string Window::GetTitle() const {
	if (IsClosed()) { return nullptr; }
	char data[256];
	GetWindowTextA(m_handle, data, sizeof(data));
	return data;
}


void Window::SetIcon(const std::string& imageFilePath) {
	if (IsClosed()) { return; }

	HANDLE hIcon = LoadImageA(0, imageFilePath.c_str(), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
	if (hIcon) {
		if (m_icon) {
			DestroyIcon((HICON)m_icon);
		}
		m_icon = hIcon;

		// Change both icons to the same icon handle.
		PostMessage(m_handle, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		PostMessage(m_handle, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

		// This will ensure that the application icon gets changed too.
		PostMessage(GetWindow(m_handle, GW_OWNER), WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		PostMessage(GetWindow(m_handle, GW_OWNER), WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	}
}


bool Window::CallEvents() {
	MessageLoopPeek();
	return false;
}


LRESULT __stdcall Window::WndProc(WindowHandle hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	void* pInstance = (void*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	Window& instance = *reinterpret_cast<Window*>(pInstance);

	auto CallClickEvent = [&instance, lParam](eMouseButton btn, eKeyState state) {
		MouseButtonEvent evt;

		evt.x = LOWORD(lParam);
		evt.y = HIWORD(lParam);
		evt.state = state;
		evt.button = btn;
		instance.CallEvent(instance.OnMouseButton, evt);
	};

	LRESULT lret;
	bool callDwp = true;
	callDwp = !DwmDefWindowProc(hwnd, msg, wParam, lParam, &lret);

	switch (msg) {
		case WM_NCCREATE:
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)((CREATESTRUCT*)lParam)->lpCreateParams);
			return TRUE;
		case WM_CLOSE:
			DestroyWindow(hwnd);
			break;
		case WM_DESTROY:
			instance.CallEvent(instance.OnClose);
			PostQuitMessage(0);
			instance.m_handle = nullptr;
			break;
		case WM_NCCALCSIZE:
			if (instance.m_borderless && wParam == TRUE) {
				NCCALCSIZE_PARAMS *pncsp = reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);
				pncsp->rgrc[2] = pncsp->rgrc[1];
				pncsp->rgrc[1] = pncsp->rgrc[0];
				if (instance.IsMaximized()) {
					pncsp->rgrc[0].left = pncsp->rgrc[0].left + instance.m_frameMargins.left;
					pncsp->rgrc[0].top = pncsp->rgrc[0].top + instance.m_frameMargins.top;
					pncsp->rgrc[0].right = pncsp->rgrc[0].right - instance.m_frameMargins.right;
					pncsp->rgrc[0].bottom = pncsp->rgrc[0].bottom - instance.m_frameMargins.bottom;
				}
				else {
					pncsp->rgrc[0].left = pncsp->rgrc[0].left + 0;
					pncsp->rgrc[0].top = pncsp->rgrc[0].top + 0;
					pncsp->rgrc[0].right = pncsp->rgrc[0].right - 0;
					pncsp->rgrc[0].bottom = pncsp->rgrc[0].bottom - 0;
				}
				return 0;
			}
			else {
				return DefWindowProc(hwnd, msg, wParam, lParam);
			}
		case WM_NCHITTEST:
			if (instance.m_borderless) {
				Vec2i cursorPos = { LOWORD(lParam), HIWORD(lParam) };
				Vec2i windowPos = instance.GetPosition();
				cursorPos -= windowPos;
				if (instance.m_captionButtonHandler) {
					instance.m_mouseHover = instance.m_captionButtonHandler(cursorPos);
				}
				int hitCategory = instance.DwmHittest(cursorPos);
				return hitCategory;
			}
			else {
				return DefWindowProc(hwnd, msg, wParam, lParam);
			}
		case WM_CHAR:
			instance.CallEvent(instance.OnCharacter, (char32_t)wParam);
			break;
		case WM_KEYDOWN: {
			KeyboardEvent evt;
			evt.key = impl::TranslateKey((unsigned)wParam);
			evt.state = eKeyState::DOWN;
			evt.repcount = LOWORD(lParam);
			if (evt.key != eKey::UNKNOWN) {
				instance.CallEvent(instance.OnKeyboard, evt);
			}
			break;
		}
		case WM_KEYUP: {
			KeyboardEvent evt;
			evt.key = impl::TranslateKey((unsigned)wParam);
			evt.state = eKeyState::UP;
			if (evt.key != eKey::UNKNOWN) {
				instance.CallEvent(instance.OnKeyboard, evt);
			}
			break;
		}
		case WM_NCLBUTTONDOWN:
			if (!instance.m_borderless) return DefWindowProc(hwnd, msg, wParam, lParam);
		case WM_LBUTTONDOWN: {
			CallClickEvent(eMouseButton::LEFT, eKeyState::DOWN);
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
		case WM_NCLBUTTONUP:
			if (!instance.m_borderless) return DefWindowProc(hwnd, msg, wParam, lParam);
			if (instance.m_mouseHover == eWindowCaptionButton::MINIMIZE) {
				instance.Minize();
			}
			if (instance.m_mouseHover == eWindowCaptionButton::MAXIMIZE) {
				instance.IsMaximized() ? instance.Restore() : instance.Maximize();
			}
		case WM_LBUTTONUP: {
			CallClickEvent(eMouseButton::LEFT, eKeyState::UP);
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
		case WM_NCLBUTTONDBLCLK:
			if (!instance.m_borderless) return DefWindowProc(hwnd, msg, wParam, lParam);
		case WM_LBUTTONDBLCLK: {
			CallClickEvent(eMouseButton::LEFT, eKeyState::DOUBLE);
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
		case WM_NCRBUTTONDOWN:
			if (!instance.m_borderless) return DefWindowProc(hwnd, msg, wParam, lParam);
		case WM_RBUTTONDOWN: {
			CallClickEvent(eMouseButton::RIGHT, eKeyState::DOWN);
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
		case WM_NCRBUTTONUP:
			if (!instance.m_borderless) return DefWindowProc(hwnd, msg, wParam, lParam);
		case WM_RBUTTONUP: {
			CallClickEvent(eMouseButton::RIGHT, eKeyState::UP);
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
		case WM_NCRBUTTONDBLCLK:
			if (!instance.m_borderless) return DefWindowProc(hwnd, msg, wParam, lParam);
		case WM_RBUTTONDBLCLK: {
			CallClickEvent(eMouseButton::RIGHT, eKeyState::DOUBLE);
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
		case WM_NCMBUTTONDOWN:
			if (!instance.m_borderless) return DefWindowProc(hwnd, msg, wParam, lParam);
		case WM_MBUTTONDOWN: {
			CallClickEvent(eMouseButton::MIDDLE, eKeyState::DOWN);
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
		case WM_NCMBUTTONUP:
			if (!instance.m_borderless) return DefWindowProc(hwnd, msg, wParam, lParam);
		case WM_MBUTTONUP: {
			CallClickEvent(eMouseButton::MIDDLE, eKeyState::UP);
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
		case WM_NCMBUTTONDBLCLK:
			if (!instance.m_borderless) return DefWindowProc(hwnd, msg, wParam, lParam);
		case WM_MBUTTONDBLCLK: {
			CallClickEvent(eMouseButton::MIDDLE, eKeyState::DOUBLE);
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
		case WM_NCXBUTTONDOWN:
			if (!instance.m_borderless) return DefWindowProc(hwnd, msg, wParam, lParam);
		case WM_XBUTTONDOWN: {
			eMouseButton btn = HIWORD(wParam) == 1 ? eMouseButton::EXTRA1 : eMouseButton::EXTRA2;
			CallClickEvent(btn, eKeyState::DOWN);
			return TRUE;
		}
		case WM_NCXBUTTONUP:
			if (!instance.m_borderless) return DefWindowProc(hwnd, msg, wParam, lParam);
		case WM_XBUTTONUP: {
			eMouseButton btn = HIWORD(wParam) == 1 ? eMouseButton::EXTRA1 : eMouseButton::EXTRA2;
			CallClickEvent(btn, eKeyState::UP);
			return TRUE;
		}
		case WM_NCXBUTTONDBLCLK:
			if (!instance.m_borderless) return DefWindowProc(hwnd, msg, wParam, lParam);
		case WM_XBUTTONDBLCLK: {
			eMouseButton btn = HIWORD(wParam) == 1 ? eMouseButton::EXTRA1 : eMouseButton::EXTRA2;
			CallClickEvent(btn, eKeyState::DOUBLE);
			return TRUE;
		}
		case WM_NCMOUSELEAVE:
			if (!instance.m_borderless) return DefWindowProc(hwnd, msg, wParam, lParam);
		case WM_MOUSELEAVE:
			instance.m_lastMouseX = -1000000;
			instance.m_lastMouseY = -1000000;
			break;
		case WM_NCMOUSEMOVE:
			if (!instance.m_borderless) return DefWindowProc(hwnd, msg, wParam, lParam);
		case WM_MOUSEMOVE: {
			MouseMoveEvent evt;
			evt.absx = GET_X_LPARAM(lParam);
			evt.absy = GET_Y_LPARAM(lParam);
			evt.relx = evt.rely = 0;
			if (instance.m_lastMouseX > -999999) {
				evt.relx = evt.absx - instance.m_lastMouseX;
				evt.rely = evt.absy - instance.m_lastMouseY;
			}
			instance.m_lastMouseX = evt.absx;
			instance.m_lastMouseY = evt.absy;
			instance.CallEvent(instance.OnMouseMove, evt);
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
		case WM_MOUSEWHEEL: {
			signed short rot = static_cast<signed short>(HIWORD(wParam));
			MouseWheelEvent evt;
			evt.rotation = (float)rot / (float)WHEEL_DELTA;
			instance.CallEvent(instance.OnMouseWheel, evt);
			break;
		}
		case WM_NCACTIVATE:
			if (!instance.m_borderless) return DefWindowProc(hwnd, msg, wParam, lParam);
		case WM_ACTIVATE: {
			instance.CallEvent(instance.OnFocus);
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
		case WM_SIZE: {
			ResizeEvent evt;
			evt.size = instance.GetSize();
			evt.clientSize = instance.GetClientSize();
			evt.resizeMode = (eResizeMode)wParam;
			instance.CallEvent(instance.OnResize, evt);
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
		case WM_NCPAINT:
			if (!instance.m_borderless) return DefWindowProc(hwnd, msg, wParam, lParam);
		case WM_PAINT: {
			PAINTSTRUCT ps;
			BeginPaint(instance.m_handle, &ps);			
			instance.CallEvent(instance.OnPaint);			
			EndPaint(instance.m_handle, &ps);
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
		default:
			if (callDwp) {
				return DefWindowProc(hwnd, msg, wParam, lParam);
			}
			else {
				return lret;
			}
	}
	return 0;
}


void Window::MessageLoop() {
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}


void Window::MessageLoopPeek() {
	MSG msg;
	while (PeekMessage(&msg, m_handle, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}


// crap'n'drop
HRESULT __stdcall Window::QueryInterface(REFIID riid, void **ppv) {
	if (riid == IID_IUnknown || riid == IID_IDropTarget) {
		*ppv = static_cast<IUnknown*>(this);
		AddRef();
		return S_OK;
	}
	*ppv = NULL;
	return E_NOINTERFACE;
}

ULONG __stdcall Window::AddRef() {
	return ++m_refCount;
}

ULONG __stdcall Window::Release() {
	LONG c = --m_refCount;
	assert(false); // you should never release this object
	return c;
}

HRESULT __stdcall Window::DragEnter(IDataObject *pdto, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect) {
	m_currentDragDropEvent = {};

	FORMATETC textFormat = { CF_UNICODETEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };;
	FORMATETC fileFormat = { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

	STGMEDIUM medium;

	// Drop text data
	if (pdto->GetData(&textFormat, &medium) == S_OK)
	{
		// We need to lock the HGLOBAL handle because we can't be sure if this is GMEM_FIXED (i.e. normal heap) data or not
		wchar_t* text = (wchar_t*)GlobalLock(medium.hGlobal);

		m_currentDragDropEvent.text = std::string(text, text + wcslen(text));

		CallEvent(OnDropEnter, m_currentDragDropEvent);

		GlobalUnlock(medium.hGlobal);
		ReleaseStgMedium(&medium);
	}
	else if (pdto->GetData(&fileFormat, &medium) == S_OK)
	{
		int fileCount = DragQueryFile((HDROP)medium.hGlobal, 0xFFFFFFFF, nullptr, 0);

		std::vector<std::filesystem::path> filePaths;
		for (int i = 0; i < fileCount; ++i)
		{
			int FileNameLength = DragQueryFileA((HDROP)medium.hGlobal, i, nullptr, 0);
			std::vector<char> fileName(FileNameLength + 1);
			DragQueryFileA((HDROP)medium.hGlobal, i, fileName.data(), FileNameLength + 1);
			filePaths.push_back(fileName.data());
		}

		m_currentDragDropEvent.filePaths = std::move(filePaths);

		CallEvent(OnDropEnter, m_currentDragDropEvent);

		ReleaseStgMedium(&medium);
	}

	*pdwEffect &= DROPEFFECT_COPY;
	return S_OK;
}

HRESULT __stdcall Window::DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect) {
	CallEvent(OnDropHover, m_currentDragDropEvent);

	*pdwEffect &= DROPEFFECT_COPY;
	return S_OK;
}

HRESULT __stdcall Window::DragLeave() {
	CallEvent(OnDropLeave, m_currentDragDropEvent);
	return S_OK;
}

HRESULT __stdcall Window::Drop(IDataObject *pdto, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect) {
	CallEvent(OnDrop, m_currentDragDropEvent);

	*pdwEffect &= DROPEFFECT_COPY;
	return S_OK;
}


} // namespace inl