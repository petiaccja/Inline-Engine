// Window implementation on [Windows OS]
#pragma once

#include "../WindowCommon.hpp"

#define NOMINMAX
#include <windows.h>
#include <queue>
#include <functional>

#define min(a,b) a < b ? a : b
#define max(a,b) a > b ? a : b
#include <gdiplus.h>
#undef min
#undef max


struct WindowDesc
{
	WindowDesc() : style(eWindowStyle::DEFAULT), clientSize(0, 0) {}

	std::string	 capText;
	eWindowStyle style;
	Vector2u	 clientSize;
	std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> userWndProc;
};

class Window
{
public:
	Window(const WindowDesc& d);

	bool PopEvent(WindowEvent& evt_out);

	void Close();
	void MinimizeSize();
	void MaximizeSize();
	void RestoreSize();


	void SetRect(const Vector2i& pos, const Vector2i& size);
	void SetPos(const Vector2i& pos);
	void SetSize(const Vector2u& size);

	void SetTitle(const std::wstring& text);

	// Getters
	bool IsOpen() const;
	bool IsFocused() const;
	bool IsMaximizedSize() const;
	bool IsMinimizedSize() const;

	size_t GetHandle() const;

	uint32_t GetClientWidth() const;
	uint32_t GetClientHeight() const;
	Vector2u GetClientSize() const;
	Vector2i GetClientCursorPos() const;

	unsigned GetNumClientPixels() const;
	float GetClientAspectRatio() const;

	Vector2i GetCenterPos() const;

	const std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)>& GetUserWndProc() const { return userWndProc; }

public:
	// HEKK
	Delegate<void()> hekkOnPaint;

	Delegate<void(WindowEvent&)>	onMousePressed;
	Delegate<void(WindowEvent&)>	onMouseReleased;
	Delegate<void(WindowEvent&)>	onMouseMoved;
	Delegate<void(Vector2u)>		onClientSizeChanged;

protected:
	friend LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	eKey ConvertFromWindowsKey(WPARAM key);

protected:
	HWND handle;
	bool bClosed;
	std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> userWndProc;
};