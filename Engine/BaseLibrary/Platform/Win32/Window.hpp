#pragma once

#include <InlineMath.hpp>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <filesystem>
#include <vector>

#include "../../Event.hpp"
#include "../../Rect.hpp"
#include "InputEvents.hpp"
#include "Input.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include "oleidl.h"
#undef DELETE


namespace inl {


using WindowHandle = HWND;

struct DragDropEvent {
	std::string text;
	std::vector<std::filesystem::path> filePaths;
};

enum class eWindowCaptionButton {
	NONE,
	BAR,
	MINIMIZE,
	MAXIMIZE,
	CLOSE,
};


class Window : private IDropTarget {
public:
	Window(const std::string& title = "Untitled",
		Vec2u size = { 640, 480 },
		bool borderless = false, 
		bool resizable = true,
		bool hiddenInitially = false);
	Window(const Window&) = delete;
	Window(Window&& rhs) noexcept;
	Window& operator=(const Window&) = delete;
	Window& operator=(Window&& rhs) noexcept;
	~Window();

	// Common
	bool IsClosed() const;
	void Close();
	void Show();
	void Hide();
	bool IsFocused() const;
	WindowHandle GetNativeHandle() const;

	// Sizing
	void Maximize();
	void Minize();
	void Restore();
	bool IsMaximized() const;
	bool IsMinimized() const;

	void SetSize(const Vec2u& size);
	Vec2u GetSize() const;
	void SetPosition(const Vec2i& position);
	Vec2i GetPosition() const;
	Vec2u GetClientSize() const;
	Vec2i GetClientCursorPos() const;

	// Borderless windows
	void SetFrameMargins(RectI frameMargins);
	RectI GetFrameMargins() const;
	void SetCaptionButtonHandler(std::function<eWindowCaptionButton(Vec2i)> handler);
	std::function<eWindowCaptionButton(Vec2i)> GetCaptionButtonHandler() const;

	// Text & style
	void SetResizable(bool enabled);
	bool GetResizable() const;
	void SetBorderless(bool enabled);
	bool GetBorderless() const;
	void SetTitle(const std::string& text);
	std::string GetTitle() const;
	void SetIcon(const std::string& imageFilePath);

	/// <summary> Calls all queued events synchronously on the caller's thread. </summary>
	/// <returns> False if some events were dropped due to too small queue size. </returns>
	bool CallEvents();
	
public:
	Event<MouseButtonEvent> OnMouseButton;
	Event<MouseMoveEvent> OnMouseMove;
	Event<MouseWheelEvent> OnMouseWheel;
	Event<KeyboardEvent> OnKeyboard;
	Event<ResizeEvent> OnResize; /// <summary> Parameters: window size, client area size. </summary>
	Event<char32_t> OnCharacter;
	Event<> OnClose;
	Event<> OnFocus;
	Event<> OnPaint;
	Event<DragDropEvent> OnDrop;
	Event<DragDropEvent> OnDropEnter;
	Event<DragDropEvent> OnDropLeave;
	Event<DragDropEvent> OnDropHover;

private:
	static LRESULT __stdcall WndProc(WindowHandle hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void MessageLoop();
	void MessageLoopPeek();
	template <class... EventArgs>
	void CallEvent(Event<EventArgs...>& evt, EventArgs... args);

	// drag'n'drop
	HRESULT __stdcall QueryInterface(const IID& riid, void **ppv) override;
	ULONG __stdcall AddRef() override;
	ULONG __stdcall Release() override;
	HRESULT __stdcall DragEnter(IDataObject *pdto, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect) override;
	HRESULT __stdcall DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect) override;
	HRESULT __stdcall DragLeave() override;
	HRESULT __stdcall Drop(IDataObject *pdto, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect) override;
	std::atomic<ULONG> m_refCount;
	DragDropEvent m_currentDragDropEvent;

	// borderless
	int DwmHittest(Vec2i cursorPos) const;
		
private:
	// WinAPI handles
	WindowHandle m_handle = NULL;
	HANDLE m_icon = NULL;

	// Window properties
	bool m_borderless = false;
	bool m_resizable = true;
	RectI m_frameMargins = { 8,8,8,8 };
	std::function<eWindowCaptionButton(Vec2i)> m_captionButtonHandler;
	eWindowCaptionButton m_mouseHover = eWindowCaptionButton::NONE;

	// Mouse move history.
	int m_lastMouseX = -1000000, m_lastMouseY = -1000000;
};


template <class... EventArgs>
void Window::CallEvent(Event<EventArgs...>& evt, EventArgs... args) {
	evt(args...);
}



} // namespace inl