#include <BaseLibrary/Logging_All.hpp>
#include <GraphicsApi_LL/IGraphicsApi.hpp>
#include <GraphicsApi_LL/Exception.hpp>
#include <GraphicsApi_D3D12/GxapiManager.hpp>
#include <GraphicsEngine_LL/GraphicsEngine.hpp>

#include <iostream>
#include <fstream>
#include <filesystem>

#define _WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <tchar.h>
#include "QCWorld.hpp"


using std::cout;
using std::endl;

using namespace inl::gxeng;
using namespace inl::gxapi;
using inl::gxapi_dx12::GxapiManager;
using namespace std::chrono_literals;

// -----------------------------------------------------------------------------
// Globals

bool isEngineInit = false;
std::ofstream logFile;
exc::Logger logger;
exc::LogStream systemLogStream = logger.CreateLogStream("system");
exc::LogStream graphicsLogStream = logger.CreateLogStream("graphics");
std::experimental::filesystem::path logFilePath;
GraphicsEngine* pEngine = nullptr;
QCWorld* pQcWorld = nullptr;


std::string errorMessage;

// -----------------------------------------------------------------------------
// Function prototypes

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool ProcessControls(int key, bool down);
bool ProcessRawInput(RAWINPUT* raw);


// -----------------------------------------------------------------------------
// main()

int main(int argc, char* argv[]) {
	// Initialize logger
	logFile.open("engine_test.log");
	logFilePath = std::experimental::filesystem::current_path();
	logFilePath /= "engine_test.log";
	cout << "Log files can be found at:\n   ";
	cout << "   " << logFilePath << endl;

	if (logFile.is_open()) {
		logger.OpenStream(&logFile);
	}
	else {
		logger.OpenStream(&std::cout);
	}

	// Set exception handler
	std::set_terminate([]()
	{
		try {
			std::rethrow_exception(std::current_exception());
			systemLogStream.Event(std::string("Terminate called, shutting down services."));
		}
		catch (std::exception& ex) {
			systemLogStream.Event(std::string("Terminate called, shutting down services.") + ex.what());
		}
		logger.Flush();
		logger.OpenStream(nullptr);
		logFile.close();
		int ans = MessageBoxA(NULL, "Open logs?", "Unhandled exception", MB_YESNO);
		if (ans == IDYES) {
			system(logFilePath.string().c_str());
		}

		std::abort();
	});


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

	// Create GraphicsEngine
	systemLogStream.Event("Initializing Graphics Engine...");
	std::unique_ptr<IGxapiManager> gxapiMgr;
	struct ReportDeleter {
		void operator()(IGraphicsApi* obj) const {
			obj->ReportLiveObjects();
			delete obj;
		}
	};
	std::unique_ptr<IGraphicsApi, ReportDeleter> gxapi;
	std::unique_ptr<GraphicsEngine> engine;
	std::unique_ptr<QCWorld> qcWorld;
	try {
		// Create manager
		systemLogStream.Event("Creating GxApi Manager...");
		gxapiMgr.reset(new GxapiManager());
		auto adapters = gxapiMgr->EnumerateAdapters();
		std::string cardList;
		for (auto adapter : adapters) {
			cardList += "\n";
			cardList += adapter.name;
		}
		systemLogStream.Event("Available graphics cards:" + cardList);


		// Create graphics api
		int device = 0;
		if (argc == 3 && argv[1] == std::string("--device") && isdigit(argv[2][0])) {
			device = argv[2][0] - '0'; // works for single digits, good enough, lol
		}
		systemLogStream.Event("Creating GraphicsApi...");
		gxapi.reset(gxapiMgr->CreateGraphicsApi(adapters[device].adapterId));
		std::stringstream ss;
		ss << "Using graphics card: " << adapters[device].name;
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
		desc.logger = &logger;

		engine.reset(new GraphicsEngine(desc));
		pEngine = engine.get();

		// Create mini world
		qcWorld.reset(new QCWorld(engine.get()));
		pQcWorld = qcWorld.get();

		isEngineInit = true;

		logger.Flush();
	}
	catch (inl::gxapi::Exception& ex) {
		pEngine = nullptr;
		isEngineInit = false;
		errorMessage = "Error creating GraphicsEngine: " + ex.Message();
		systemLogStream.Event(errorMessage);
		logger.Flush();
	}
	catch (std::exception& ex) {
		pEngine = nullptr;
		isEngineInit = false;
		errorMessage = std::string("Error creating GraphicsEngine: ") + ex.what();
		systemLogStream.Event(errorMessage);
		logger.Flush();
	}


	// Show the window
	InvalidateRect(hWnd, nullptr, TRUE);

	// Register for raw input
	RAWINPUTDEVICE Rid[1];
	Rid[0].usUsagePage = 0x01;
	Rid[0].usUsage = 0x02;
	Rid[0].dwFlags = 0;   // adds HID mouse and also ignores legacy mouse messages
	Rid[0].hwndTarget = 0;
	if (RegisterRawInputDevices(Rid, 1, sizeof(Rid[0])) == FALSE) {
		cout << "Raw input failed: " << GetLastError() << endl;
	}


	// Game-style main loop
	MSG msg;
	bool run = true;

	std::chrono::high_resolution_clock::time_point timestamp = std::chrono::high_resolution_clock::now();
	std::chrono::nanoseconds frameTime(1000);
	std::chrono::nanoseconds frameRateUpdate(0);
	std::vector<std::chrono::nanoseconds> frameTimeHistory;
	float avgFps = 0;

	while (run) {
		while (PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				run = false;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (qcWorld) {
			try {
				// Measure time of Update().
				auto updateStart = std::chrono::high_resolution_clock::now();

				// Update world
				qcWorld->UpdateWorld(frameTime.count() / 1e9f);
				qcWorld->RenderWorld(frameTime.count() / 1e9f);

				auto updateEnd = std::chrono::high_resolution_clock::now();
				std::chrono::nanoseconds updateElapsed = updateEnd - updateStart;

				// Calculate elapsed time for frame.
				auto now = std::chrono::high_resolution_clock::now();
				frameTime = now - timestamp;
				timestamp = now;
				//std::this_thread::sleep_for(16667us - updateElapsed);

				frameRateUpdate += frameTime;
				if (frameRateUpdate > 500ms) {
					frameRateUpdate = 0ns;

					double avgFrameTime = 0.0;
					for (auto v : frameTimeHistory) {
						avgFrameTime += v.count() / 1e9;
					}
					avgFrameTime /= frameTimeHistory.size();
					avgFps = 1 / avgFrameTime;

					frameTimeHistory.clear();
				}
				frameTimeHistory.push_back(frameTime);


				std::stringstream ss;
				unsigned width, height;
				engine->GetScreenSize(width, height);
				ss << "Graphics Engine Test | " << width << "x" << height << " | FPS=" << (int)avgFps;
				SetWindowTextA(hWnd, ss.str().c_str());
			}
			catch (std::exception& ex) {
				systemLogStream.Event(std::string("Graphics engine error: ") + ex.what());
				logger.Flush();
				PostQuitMessage(0);
			}
		}
	}

	cout << "Shutting down." << endl;
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
				DrawTextA(hdc, errorMessage.c_str(), -1, &clientRect, DT_CENTER | DT_VCENTER);
				EndPaint(hWnd, &paintStruct);
				return 0;
			}
			else {
				return DefWindowProc(hWnd, msg, wParam, lParam);
			}
		}
		case WM_SIZE:
		{
			if (isEngineInit) {
				RECT clientRect;
				GetClientRect(hWnd, &clientRect);
				unsigned width = clientRect.right - clientRect.top;
				unsigned height = clientRect.bottom - clientRect.top;
				pEngine->SetScreenSize(width, height);
				pQcWorld->SetAspectRatio((float)width / (float(height)));
				return 0;
			}
			else {
				return DefWindowProc(hWnd, msg, wParam, lParam);
			}
		}
		case WM_KEYDOWN:
		{
			if (ProcessControls(wParam, true)) {
				return 0;
			}
			return DefWindowProc(hWnd, msg, wParam, lParam);
		}
		case WM_KEYUP:
			if (wParam == VK_ESCAPE) {
				PostQuitMessage(0);
				return 0;
			}
			else if (wParam == VK_RETURN) {
				if (pEngine) {
					bool isfs = pEngine->GetFullScreen();
					if (isfs) {
						pEngine->SetFullScreen(false);
					}
					else {
						int monWidth = GetSystemMetrics(SM_CXSCREEN);
						int monHeight = GetSystemMetrics(SM_CYSCREEN);
						pEngine->SetFullScreen(true);
						pEngine->SetScreenSize(monWidth, monHeight);
					}
				}
				return 0;
			}
			else if (wParam == 'L') {
				if (pQcWorld) {
					pQcWorld->IWantSunsetBitches();
				}
				return 0;
			}
			else if (pQcWorld && ProcessControls(wParam, false)) {
				return 0;
			}
			return DefWindowProc(hWnd, msg, wParam, lParam);
		case WM_INPUT: {
			// get raw input data
			UINT dwSize;
			GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
			std::vector<LPBYTE> lpb(dwSize);
			if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb.data(), &dwSize,
				sizeof(RAWINPUTHEADER)) != dwSize)
				OutputDebugString(TEXT("GetRawInputData does not return correct size !\n"));

			RAWINPUT* raw = (RAWINPUT*)lpb.data();
			if (pQcWorld && ProcessRawInput(raw)) {
				return 0;
			}
			else {
				DefWindowProc(hWnd, msg, wParam, lParam);
			}
		}
		case WM_KILLFOCUS:
			ShowCursor(TRUE);
		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}



bool ProcessControls(int key, bool down) {
	float enable = down ? 1.f : 0.f;
	switch (key) {
		case 'W': pQcWorld->TiltForward(enable); break;
		case 'A': pQcWorld->TiltLeft(enable); break;
		case 'S': pQcWorld->TiltBackward(enable); break;
		case 'D': pQcWorld->TiltRight(enable); break;
		case VK_LEFT: pQcWorld->RotateLeft(enable); break;
		case VK_RIGHT: pQcWorld->RotateRight(enable); break;
		case VK_UP: pQcWorld->Ascend(enable); break;
		case VK_DOWN: pQcWorld->Descend(enable); break;
		default: return false;
	}
	return true;
}

bool ProcessRawInput(RAWINPUT* raw) {
	if (raw->header.dwType == RIM_TYPEMOUSE) {
		// track up/down states
		static bool lmbDown = false, mmbDown = false, rmbDown = false;
		static POINT cursorPos{ 0, 0 };
		if (raw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN) {
			lmbDown = true;
		}
		if (raw->data.mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN) {
			mmbDown = true;
		}
		if (raw->data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN) {
			rmbDown = true;
		}
		if (raw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP) {
			lmbDown = false;
		}
		if (raw->data.mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP) {
			mmbDown = false;
		}
		if (raw->data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP) {
			rmbDown = false;
		}
		bool down = mmbDown || rmbDown;
		if (down) {
			SetCursorPos(cursorPos.x, cursorPos.y);
		}
		else {
			GetCursorPos(&cursorPos);
		}

		// check if relative
		bool relative = raw->data.mouse.usFlags == 0;
		if (!relative) {
			return false;
		}

		//  set motion
		float dx = raw->data.mouse.lLastX, dy = raw->data.mouse.lLastY;
		static float tiltfw = 0.0f, tiltr = 0.0f;
		static float turnr = 0.0f;
		static float heading = 0.0f;
		static float lookoff = 0.0f;
		static float look = 0.0f;

		if (rmbDown) {
			turnr += -dx / 400.f;
			pQcWorld->Heading(heading + turnr);
		}
		else {
			heading = pQcWorld->Heading();
			turnr = 0.0f;
		}

		static bool lastMmbDown = false;
		if (mmbDown) {
			tiltfw += -dy / 300.f;
			tiltr += dx / 300.f;
			pQcWorld->TiltForward(tiltfw);
			pQcWorld->TiltRight(tiltr);
		}
		else {
			if (lastMmbDown != mmbDown) {
				pQcWorld->TiltForward(0);
				pQcWorld->TiltRight(0);
			}
			tiltfw = tiltr = 0;
		}
		lastMmbDown = mmbDown;

		if (rmbDown) {
			lookoff += -dy / 400.f;
			pQcWorld->Look(look + lookoff);
		}
		else {
			look = pQcWorld->Look();
			lookoff = 0;
		}

		return true;
	}

	return false;
}