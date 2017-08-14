// Window implementation on [Windows OS]
#pragma once

#include "../WindowCommon.hpp"
#include "WindowDropTarget.hpp"

#include <BaseLibrary/Delegate.hpp>
#include <queue>
#include <functional>

#define NOMINMAX
#include <windows.h>

// The Win32 api Window class
class Window
{
public:
	Window(const WindowDesc& d);
	~Window();

	bool PopEvent(WindowEvent& evt_out);

	void Close();
	void MinimizeSize();
	void MaximizeSize();
	void RestoreSize();


	void SetHekkTillGdiNotRemoved(const Delegate<void(Vec2& pos)>& hekk) { this->hekk = hekk; }

	void SetRect(const Vec2i& pos, const Vec2u& size);
	void SetPos(const Vec2i& pos);
	void SetSize(const Vec2u& size);

	void SetTitle(const std::string& text);
	void SetIcon(const std::wstring& filePath);

	// Getters
	bool IsOpen() const;
	bool IsFocused() const;
	bool IsMaximizedSize() const;
	bool IsMinimizedSize() const;

	WindowHandle GetHandle() const;

	uint32_t GetClientWidth() const;
	uint32_t GetClientHeight() const;
	Vec2u GetClientSize() const;
	Vec2 GetClientCursorPos() const;

	unsigned GetNumClientPixels() const;
	float GetClientAspectRatio() const;

	Vec2i GetCenterPos() const;

	const std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)>& GetUserWndProc() const { return userWndProc; }

public:
	// HEKK
	Delegate<void()> hekkOnPaint;

	Delegate<void(WindowEvent&)>	onMousePressed;
	Delegate<void(WindowEvent&)>	onMouseReleased;
	Delegate<void(WindowEvent&)>	onMouseMoved;
	Delegate<void(Vec2u)>		onClientSizeChanged;
	Delegate<void(DragData&)>		onDropped;
	Delegate<void(DragData&)>		onDragEntered;
	Delegate<void(DragData&)>		onDragLeaved;
	Delegate<void(DragData&)>		onDragHovering;
	

protected:
	friend LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	eKey ConvertFromWindowsKey(WPARAM key);

protected:
	HWND handle;
	bool bClosed;
	std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> userWndProc;
	WindowDropTarget dropTarget; // TODO later we will need this for fully functional Drag&Drop

	Delegate<void(Vec2&)> hekk;
};