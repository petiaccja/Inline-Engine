#include "PicoEngine.hpp"
#include <BaseLibrary/Logging_All.hpp>

#define _WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <tchar.h>


#include <iostream>
#include <GraphicsApi_LL/Exception.hpp>
using std::cout;
using std::endl;


// -----------------------------------------------------------------------------
// Globals

bool isEngineInit = false;
exc::Logger logger;
exc::LogStream systemLogStream = logger.CreateLogStream("system");
exc::LogStream graphicsLogStream = logger.CreateLogStream("graphics");


// -----------------------------------------------------------------------------
// Function prototypes

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);




// -----------------------------------------------------------------------------
// main()

int main() {
	// Initialize logger
	logger.OpenStream(&std::cout);


	// Create and register window class
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

	// Create the window itself
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

	// Show the window
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	// Get actual client area params
	RECT clientRect;
	GetClientRect(hWnd, &clientRect);
	int width = clientRect.right - clientRect.left;
	int height = clientRect.bottom - clientRect.top;

	// Create PicoEngine
	systemLogStream.Event("Initializing PicoEngine...");
	std::unique_ptr<PicoEngine> engine;
	try {
		engine.reset(new PicoEngine(hWnd, width, height, &graphicsLogStream));
		isEngineInit = true;
		
		logger.Flush();
	}
	catch (inl::gxapi::Exception& ex) {
		isEngineInit = false;

		systemLogStream.Event("Error creating PicoEngine: " + ex.Message());
		logger.Flush();
	}
	catch (std::exception& ex) {
		isEngineInit = false;
		
		systemLogStream.Event(std::string("Error creating PicoEngine: ") + ex.what());
		logger.Flush();
	}


	// Game-style main loop
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
		if (engine) {
			engine->Update();
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
			if (!isEngineInit) {
				HDC hdc;
				PAINTSTRUCT paintStruct;
				hdc = BeginPaint(hWnd, &paintStruct);
				RECT clientRect;
				GetClientRect(hWnd, &clientRect);
				FillRect(hdc, &clientRect, CreateSolidBrush(RGB(192, 0, 0)));
				DrawTextA(hdc, " Initialize Device! ", -1, &clientRect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
				EndPaint(hWnd, &paintStruct);
				return 0;
			}
			else {
				return DefWindowProc(hWnd, msg, wParam, lParam);
			}
		}
		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}
