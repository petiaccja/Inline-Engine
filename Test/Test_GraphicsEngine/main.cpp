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
		TEXT("Graphics Engine Test"),
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

		systemLogStream.Event("Error creating GraphicsEngine: " + ex.Message());
		logger.Flush();
	}
	catch (std::exception& ex) {
		isEngineInit = false;
		
		systemLogStream.Event(std::string("Error creating GraphicsEngine: ") + ex.what());
		logger.Flush();
	}


	// Game-style main loop
	MSG msg;
	bool run = true;
	std::chrono::high_resolution_clock::time_point timestamp = std::chrono::high_resolution_clock::now();
	float fpsHistory[10] = {0};
	unsigned fpsHistoryIdx = 0;
	while (run) {
		while (PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				run = false;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (engine) {
			auto updateStart = std::chrono::high_resolution_clock::now();
			engine->Update(0.016f);
			auto updateEnd = std::chrono::high_resolution_clock::now();
			std::chrono::nanoseconds updateElapsed = updateEnd - updateStart;


			auto now = std::chrono::high_resolution_clock::now();
			std::chrono::nanoseconds elapsed = now - timestamp;
			timestamp = now;
			std::this_thread::sleep_for(16ms - updateElapsed);

			std::stringstream ss;
			float currentFps = 1.0 / (elapsed.count() / 1e9);
			fpsHistory[fpsHistoryIdx] = currentFps;
			fpsHistoryIdx++; fpsHistoryIdx %= 10;
			float avgFps = [&] {
				float sum = 0;
				for (auto v : fpsHistory)
					sum += v;
				return sum / 10.0f;
			}();
			ss << "Graphics Engine Test | " << "FPS=" << (int)avgFps;
			SetWindowTextA(hWnd, ss.str().c_str());
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
		case WM_KEYUP:
			PostQuitMessage(0);
			return 0;
		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}
