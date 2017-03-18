// Window implementation on [Windows OS]
#pragma once

#include "../WindowCommon.hpp"
#include <windows.h>
#include <queue>
#include <functional>

class Window
{
public:
	// TMP HEKK
	void RegOnPaint(const std::function<void()>& fn) { hekkOnPaint = fn; }

	Window(const WindowDesc& d);
	~Window();

	bool PopEvent(WindowEvent& evt_out);

	void Close();

	void Clear(const Color& color);

	void SetPos(const ivec2& pos = ivec2(0, 0));
	void SetSize(const uvec2& size);

	void SetClientPixels(const Color* const pixels);

	void SetTitle(const std::wstring& text);

	void SetCursorVisible(bool bVisible);

	// Getters
	bool IsOpen() const;

	size_t GetHandle() const;

	uint32_t GetClientWidth() const;
	uint32_t  GetClientHeight() const;

	unsigned GetNumClientPixels() const;
	float GetClientAspectRatio() const;

	ivec2 GetCenterPos() const;

public:
	// HEKK
	std::function<void()> hekkOnPaint;

protected:
	eKey	ConvertFromWindowsKey(WPARAM key);
	friend LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void PostEvent(const MSG& msg);

protected:
	HWND hwnd;
	bool bClosed;

	std::queue<MSG> wndProcMessages;
	//sf::RenderWindow w;
	//ivec2 lastMousePos;
};

// TMP HEKK
extern Window* GWindow;