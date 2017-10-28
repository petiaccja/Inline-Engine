#include "Window.hpp"
#include "BaseLibrary/Exception/Exception.hpp"
#include <future>
#include <Windowsx.h>
#include "shellapi.h"
#undef UNKNOWN

#undef IsMaximized
#undef IsMinimized


namespace inl {



Window::Window(const std::string& title,
	Vec2 size,
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

	// Message loop runs in another thread.
	// If the thread is finished setting up the window the promise is signaled.
	std::promise<void> creationPromise;
	std::future<void> creationResult = creationPromise.get_future();

	// The message loop thread.
	m_messageThread = std::thread(
		[
			this,
			creationPromise = std::move(creationPromise),
			title, size, borderless, resizable, hiddenInitially
		]() mutable {

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
			creationPromise.set_exception(std::current_exception());
			if (hwnd) {
				DestroyWindow(hwnd);
			}
			return;
		}
		creationPromise.set_value();

		// Show and update the newly created window.
		m_handle = hwnd;
		if (!hiddenInitially) {
			ShowWindow(m_handle, SW_SHOW);
		}
		UpdateWindow(m_handle);

		// Start running the WINAPI message loop.
		MessageLoop();
	});

	try {
		creationResult.get();
	}
	catch (...) {
		m_messageThread.join();
		throw;
	}
}


Window::Window(Window&& rhs) noexcept {
	m_handle = rhs.m_handle;
	m_messageThread = std::move(rhs.m_messageThread);

	rhs.m_handle = NULL;
}


Window& Window::operator=(Window&& rhs) noexcept {
	m_handle = rhs.m_handle;
	m_messageThread = std::move(rhs.m_messageThread);

	rhs.m_handle = NULL;

	return *this;
}


Window::~Window() {
	if (m_handle != 0) {
		DestroyWindow(m_handle);
	}
	if (m_messageThread.joinable()) {
		m_messageThread.join();
	}
	if (m_icon) {
		DestroyIcon((HICON)m_icon);
	}
}



bool Window::IsClosed() const {
	return m_handle == NULL;
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

void Window::SetSize(const Vec2& size) {
	if (IsClosed()) { return; }
	SetWindowPos(m_handle, NULL, 0, 0, size.x, size.y, SWP_NOMOVE);
}


Vec2 Window::GetSize() const {
	if (IsClosed()) { return { 0,0 }; }
	RECT rc;
	GetWindowRect(m_handle, &rc);
	return { rc.right - rc.left, rc.bottom - rc.top };
}


Vec2 Window::GetClientSize() const {
	if (IsClosed()) { return { 0,0 }; }
	RECT rc;
	GetClientRect(m_handle, &rc);
	return { rc.right - rc.left, rc.bottom - rc.top };
}


void Window::SetPosition(const Vec2& position) {
	if (IsClosed()) { return; }
	SetWindowPos(m_handle, NULL, position.x, position.y, 0, 0, SWP_NOMOVE);
}


Vec2 Window::GetPosition() const {
	if (IsClosed()) { return { 0,0 }; }
	RECT rc;
	GetWindowRect(m_handle, &rc);
	return { rc.left, rc.top };
}


void Window::SetResizable(bool enabled) {
	throw NotImplementedException();
	if (IsClosed()) { return; }
}


bool Window::GetResizable() const {
	throw NotImplementedException();
	if (IsClosed()) { return false; }
}


void Window::SetBorderless(bool enabled) {
	throw NotImplementedException();
	if (IsClosed()) { return; }
}


bool Window::GetBorderless() const {
	throw NotImplementedException();
	if (IsClosed()) { return false; }
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


void Window::SetQueueSizeHint(size_t queueSize) {
	m_queueSize = queueSize;
}


void Window::SetQueueMode(eInputQueueMode mode) {
	m_queueMode = mode;
}


bool Window::CallEvents() {
	std::unique_lock<decltype(m_queueMtx)> lk(m_queueMtx);

	bool eventDropped = m_eventDropped;
	m_eventDropped = false;
	std::queue<std::function<void()>> eventQueue = std::move(m_eventQueue);

	lk.unlock();

	while (!eventQueue.empty()) {
		auto functor = eventQueue.front();
		eventQueue.pop();
		functor();
	}

	return eventDropped;
}


eInputQueueMode Window::GetQueueMode() const {
	return m_queueMode;
}


LRESULT __stdcall Window::WndProc(WindowHandle hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	Window& instance = *reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

	auto CallClickEvent = [&instance, lParam](eMouseButton btn, eKeyState state) {
		MouseButtonEvent evt;

		evt.x = LOWORD(lParam);
		evt.y = HIWORD(lParam);
		evt.state = state;
		evt.button = btn;
		instance.CallEvent(instance.OnMouseButton, evt);
	};

	switch (msg) {
		case WM_CLOSE:
			DestroyWindow(hwnd);
			return 0;
		case WM_DESTROY:
			instance.CallEvent(instance.OnClose);
			PostQuitMessage(0);
			return 0;
		case WM_CHAR:
			instance.CallEvent(instance.OnCharacter, (char32_t)wParam);
			return 0;
		case WM_KEYDOWN: {
			KeyboardEvent evt;
			evt.key = impl::TranslateKey(wParam);
			evt.state = eKeyState::DOWN;
			if (evt.key != eKey::UNKNOWN) {
				instance.CallEvent(instance.OnKeyboard, evt);
			}
			return 0;
		}
		case WM_KEYUP: {
			KeyboardEvent evt;
			evt.key = impl::TranslateKey(wParam);
			evt.state = eKeyState::UP;
			if (evt.key != eKey::UNKNOWN) {
				instance.CallEvent(instance.OnKeyboard, evt);
			}
			return 0;
		}
		case WM_LBUTTONDOWN: {
			CallClickEvent(eMouseButton::LEFT, eKeyState::DOWN);
			return 0;
		}
		case WM_LBUTTONUP: {
			CallClickEvent(eMouseButton::LEFT, eKeyState::UP);
			return 0;
		}
		case WM_LBUTTONDBLCLK: {
			CallClickEvent(eMouseButton::LEFT, eKeyState::DOUBLE);
			return 0;
		}
		case WM_RBUTTONDOWN: {
			CallClickEvent(eMouseButton::RIGHT, eKeyState::DOWN);
			return 0;
		}
		case WM_RBUTTONUP: {
			CallClickEvent(eMouseButton::RIGHT, eKeyState::UP);
			return 0;
		}
		case WM_RBUTTONDBLCLK: {
			CallClickEvent(eMouseButton::RIGHT, eKeyState::DOUBLE);
			return 0;
		}
		case WM_MBUTTONDOWN: {
			CallClickEvent(eMouseButton::MIDDLE, eKeyState::DOWN);
			return 0;
		}
		case WM_MBUTTONUP: {
			CallClickEvent(eMouseButton::MIDDLE, eKeyState::UP);
			return 0;
		}
		case WM_MBUTTONDBLCLK: {
			CallClickEvent(eMouseButton::MIDDLE, eKeyState::DOUBLE);
			return 0;
		}
		case WM_XBUTTONDOWN: {
			eMouseButton btn = HIWORD(wParam) == 1 ? eMouseButton::EXTRA1 : eMouseButton::EXTRA2;
			CallClickEvent(btn, eKeyState::DOWN);
		}
		case WM_XBUTTONUP: {
			eMouseButton btn = HIWORD(wParam) == 1 ? eMouseButton::EXTRA1 : eMouseButton::EXTRA2;
			CallClickEvent(btn, eKeyState::UP);
		}
		case WM_XBUTTONDBLCLK: {
			eMouseButton btn = HIWORD(wParam) == 1 ? eMouseButton::EXTRA1 : eMouseButton::EXTRA2;
			CallClickEvent(btn, eKeyState::DOUBLE);
		}
		case WM_MOUSEMOVE: {
			MouseMoveEvent evt;
			evt.absx = GET_X_LPARAM(lParam);
			evt.absy = GET_Y_LPARAM(lParam);
			evt.relx = evt.rely = 0;
			instance.CallEvent(instance.OnMouseMove, evt);
			return 0;
		}
		case WM_MOUSEWHEEL: {
			short rot = static_cast<short>(HIWORD(wParam));
			MouseButtonEvent evt;
			// nahh this does not work
			return 0;
		}
		case WM_ACTIVATE: {
			instance.CallEvent(instance.OnFocus);
			return 0;
		}
		case WM_SIZE: {
			instance.CallEvent(instance.OnResize, instance.GetSize(), instance.GetClientSize());
			return 0;
		}
		case WM_NCCREATE:
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)((CREATESTRUCT*)lParam)->lpCreateParams);
			return DefWindowProc(hwnd, msg, wParam, lParam);
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}


void Window::MessageLoop() {
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
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

		CallEvent(OnDropEntered, m_currentDragDropEvent);

		GlobalUnlock(medium.hGlobal);
		ReleaseStgMedium(&medium);
	}
	else if (pdto->GetData(&fileFormat, &medium) == S_OK)
	{
		int fileCount = DragQueryFile((HDROP)medium.hGlobal, 0xFFFFFFFF, nullptr, 0);

		std::vector<std::experimental::filesystem::path> filePaths;
		for (int i = 0; i < fileCount; ++i)
		{
			int FileNameLength = DragQueryFileA((HDROP)medium.hGlobal, i, nullptr, 0);
			std::vector<char> fileName(FileNameLength + 1);
			DragQueryFileA((HDROP)medium.hGlobal, i, fileName.data(), FileNameLength + 1);
			filePaths.push_back(fileName.data());
		}

		m_currentDragDropEvent.filePaths = std::move(filePaths);

		CallEvent(OnDropEntered, m_currentDragDropEvent);

		ReleaseStgMedium(&medium);
	}

	*pdwEffect &= DROPEFFECT_COPY;
	return S_OK;
}

HRESULT __stdcall Window::DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect) {
	CallEvent(OnDropHovering, m_currentDragDropEvent);

	*pdwEffect &= DROPEFFECT_COPY;
	return S_OK;
}

HRESULT __stdcall Window::DragLeave() {
	CallEvent(OnDropLeft, m_currentDragDropEvent);
	return S_OK;
}

HRESULT __stdcall Window::Drop(IDataObject *pdto, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect) {
	CallEvent(OnDropped, m_currentDragDropEvent);

	*pdwEffect &= DROPEFFECT_COPY;
	return S_OK;
}


} // namespace inl