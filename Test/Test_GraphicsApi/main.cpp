#define _WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <tchar.h>


#include <iostream>
using std::cout;
using std::endl;


LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


int main() {
	WNDCLASSEX wc;
	wc.cbClsExtra = 0;
	wc.cbSize = sizeof(wc);
	wc.cbWndExtra = 0;
	wc.hbrBackground = NULL;
	wc.hCursor = NULL;
	wc.hIcon = NULL;
	wc.hIconSm = NULL;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = TEXT("WC_GXAPITEST");
	wc.lpszMenuName = TEXT("MENUNAME");
	wc.style = CS_VREDRAW | CS_HREDRAW;
	ATOM r = RegisterClassEx(&wc);
	if (r == FALSE) {
		cout << "Could not register window class." << endl;
		return 0;
	}
	HWND hWnd = CreateWindowEx(
		NULL,
		TEXT("WC_GXAPITEST"),
		TEXT("GxApi Test, bitches!"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		800,
		600,
		NULL,
		NULL,
		wc.hInstance,
		NULL);

	if (hWnd == INVALID_HANDLE_VALUE) {
		cout << "Could not create window." << endl;
		return 0;
	}

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	RECT clientRect;
	GetClientRect(hWnd, &clientRect);


	MSG msg;
	bool run = true;
	while (run) {
		while (PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				run = false;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return 0;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_CLOSE:
			PostQuitMessage(0);
			return 0;
		case WM_PAINT:
		{
			HDC hdc;
			PAINTSTRUCT paintStruct;
			hdc = BeginPaint(hWnd, &paintStruct);
			RECT clientRect;
			GetClientRect(hWnd, &clientRect);
			FillRect(hdc, &clientRect, CreateSolidBrush(RGB(192, 0, 0)));
			DrawTextA(hdc, "Initialize Device!", -1, &clientRect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
			EndPaint(hWnd, &paintStruct);
			return 0;
		}
		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}
