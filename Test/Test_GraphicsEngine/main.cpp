#include <BaseLibrary/Logging_All.hpp>
#include <GraphicsApi_LL/IGraphicsApi.hpp>
#include <GraphicsApi_LL/Exception.hpp>
#include <GraphicsApi_D3D12/GxapiManager.hpp>
#include <GraphicsEngine_LL/GraphicsEngine.hpp>

#include <iostream>

#define _WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <tchar.h>


using std::cout;
using std::endl;

using namespace inl::gxeng;
using namespace inl::gxapi;
using inl::gxapi_dx12::GxapiManager;
using namespace std::chrono_literals;

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
	wc.lpszClassName = TEXT("WC_ENGINETEST");
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
		TEXT("WC_ENGINETEST"),
		TEXT("Engine Test, bitches!"),
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
	systemLogStream.Event("Initializing Graphics Engine...");
	std::unique_ptr<IGxapiManager> gxapiMgr;
	std::unique_ptr<IGraphicsApi> gxapi;
	std::unique_ptr<GraphicsEngine> engine;
	try {
		// Create manager
		systemLogStream.Event("Creating GxApi Manager...");
		gxapiMgr.reset(new GxapiManager());
		auto adapters = gxapiMgr->EnumerateAdapters();


		// Create graphics api
		systemLogStream.Event("Creating GraphicsApi...");
		gxapi.reset(gxapiMgr->CreateGraphicsApi(adapters[0].adapterId));
		std::stringstream ss;
		ss << "Using graphics card: " << adapters[0].name;
		systemLogStream.Event(ss.str());


		// Create graphics engine
		systemLogStream.Event("Creating Graphics Engine...");

		GraphicsEngineDesc desc;
		desc.fullScreen = false;
		desc.graphicsApi = gxapi.get();
		desc.gxapiManager = gxapiMgr.get();
		desc.width = width;
		desc.height = height;
		desc.targetWindow = hWnd;

		engine.reset(new GraphicsEngine(desc));
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
			engine->Update(0.016f);
			std::this_thread::sleep_for(16ms);
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
				FillRect(hdc, &clientRect, CreateSolidBrush(RGB(64, 96, 192)));
				DrawTextA(hdc, " Initialize Engine! ", -1, &clientRect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
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
