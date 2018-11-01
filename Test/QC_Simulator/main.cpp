// base lib 
#include <BaseLibrary/Logging_All.hpp>
#include <BaseLibrary/Platform/System.hpp>
#include <BaseLibrary/Platform/Input.hpp>
#include <BaseLibrary/Platform/Window.hpp>
#include <BaseLibrary/Timer.hpp>

// include interfaces
#include <GraphicsApi_LL/IGraphicsApi.hpp>
#include <GraphicsApi_LL/HardwareCapability.hpp>
#include <GraphicsApi_LL/Exception.hpp>

// hacked includes - should use some factory
#include <GraphicsApi_D3D12/GxapiManager.hpp>
#include <GraphicsEngine_LL/GraphicsEngine.hpp>

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>

#include "QCWorld.hpp"


using std::cout;
using std::endl;

using namespace inl;
using namespace inl::gxeng;
using namespace inl::gxapi;
using inl::gxapi_dx12::GxapiManager;
using namespace std::chrono_literals;

// -----------------------------------------------------------------------------
// Globals

std::ofstream logFile;
Logger logger;
LogStream systemLogStream = logger.CreateLogStream("system");
LogStream graphicsLogStream = logger.CreateLogStream("graphics");
std::filesystem::path logFilePath;

std::string errorMessage;

// -----------------------------------------------------------------------------
// Function prototypes
std::string SelectPipeline(IGraphicsApi* gxapi);
void OnTerminate();


// -----------------------------------------------------------------------------
// Helper classes

// Reports live GPU object when GraphicsApi is freed
struct ReportDeleter {
	void operator()(IGraphicsApi* obj) const {
		obj->ReportLiveObjects();
		delete obj;
	}
};

// Processes key and joy input to control the copter
class InputHandler {
public:
	InputHandler(QCWorld* qcWorld) : world(qcWorld) {}

	void OnJoystickMove(JoystickMoveEvent evt);
	void OnKey(KeyboardEvent evt);
	void SetFocused(bool focused) { m_focused = focused; }
	void OnResize(ResizeEvent);
private:
	QCWorld* world;
	bool m_focused = true;
};


// -----------------------------------------------------------------------------
// main()

int main(int argc, char* argv[]) {
	// Initialize logger
	logFile.open("engine_test.log");
	logFilePath = std::filesystem::current_path();
	logFilePath /= "engine_test.log";
	cout << "Log files can be found at:\n   ";
	cout << "   " << logFilePath << endl;

	if (logFile.is_open()) {
		logger.OpenStream(&logFile);
	}
	else {
		logger.OpenStream(&std::cout);
	}

	// Set exception terminate handler
	std::set_terminate(OnTerminate);


	// Create the window itself
	Window window;
	window.SetTitle("QC Simulator");
	window.SetSize({ 1024, 576 });


	// Create GraphicsEngine
	systemLogStream.Event("Initializing Graphics Engine...");

	std::unique_ptr<IGxapiManager> gxapiMgr;
	std::unique_ptr<IGraphicsApi, ReportDeleter> gxapi;
	std::unique_ptr<GraphicsEngine> engine;
	std::unique_ptr<QCWorld> qcWorld;
	std::unique_ptr<InputHandler> inputHandler;
	std::unique_ptr<Input> joyInput;
	std::unique_ptr<Input> keyboardInput;

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
		desc.width = window.GetClientSize().x;
		desc.height = window.GetClientSize().y;
		desc.targetWindow = window.GetNativeHandle();
		desc.logger = &logger;

		engine.reset(new GraphicsEngine(desc));

		// Load graphics pipeline
		std::string pipelineFileName = SelectPipeline(gxapi.get());
		std::ifstream pipelineFile(INL_GAMEDATA "\\Pipelines\\" + pipelineFileName);
		if (!pipelineFile.is_open()) {
			throw FileNotFoundException("Failed to open pipeline JSON.");
		}
		std::string pipelineDesc((std::istreambuf_iterator<char>(pipelineFile)), std::istreambuf_iterator<char>());
		engine->LoadPipeline(pipelineDesc);


		// Create mini world
		qcWorld.reset(new QCWorld(engine.get()));
		

		// Create input handling
		inputHandler = std::make_unique<InputHandler>(qcWorld.get());

		window.OnResize += Delegate<void(ResizeEvent)>{ &InputHandler::OnResize, inputHandler.get() };

		auto joysticks = Input::GetDeviceList(eInputSourceType::JOYSTICK);
		if (!joysticks.empty()) {
			joyInput = std::make_unique<Input>(joysticks.front().id);
			joyInput->SetQueueMode(eInputQueueMode::QUEUED);
			joyInput->OnJoystickMove += Delegate<void(JoystickMoveEvent)>{ &InputHandler::OnJoystickMove, inputHandler.get() };
		}
		auto keyboards = Input::GetDeviceList(eInputSourceType::KEYBOARD);
		if (!keyboards.empty()) {
			keyboardInput = std::make_unique<Input>(keyboards.front().id);
			keyboardInput->SetQueueMode(eInputQueueMode::QUEUED);
			keyboardInput->OnKeyboard += Delegate<void(KeyboardEvent)>{ &InputHandler::OnKey, inputHandler.get() };
		}

		window.OnResize += [&engine, &qcWorld](ResizeEvent evt) {
			engine->SetScreenSize(evt.clientSize.x, evt.clientSize.y);
			qcWorld->ScreenSizeChanged(evt.clientSize.x, evt.clientSize.y);
		};

		logger.Flush();
	}
	catch (Exception& ex) {
		errorMessage = std::string("Error creating GraphicsEngine: ") + ex.what() + "\n" + ex.StackTraceStr();
		systemLogStream.Event(errorMessage);
		logger.Flush();
	}
	catch (std::exception& ex) {
		errorMessage = std::string("Error creating GraphicsEngine: ") + ex.what();
		systemLogStream.Event(errorMessage);
		logger.Flush();
	}

	if (!qcWorld) {
		return 0;
	}


	// Main rendering loop
	Timer timer;
	timer.Start();
	double frameTime = 0.05, frameRateUpdate = 0;
	std::vector<double> frameTimeHistory;
	float avgFps = 0;

	auto CaptionHandler = [&window](Vec2i cursorPos) {
		Vec2i size = window.GetSize();
		RectI rc;
		rc.top = 5;
		rc.bottom = 50;
		rc.right = size.x - 5;
		rc.left = size.x - 50;
		if (rc.IsPointInside(cursorPos))
			return eWindowCaptionButton::CLOSE;
		rc.Move({ -50, 0 });
		if (rc.IsPointInside(cursorPos))
			return eWindowCaptionButton::MAXIMIZE;
		rc.Move({ -50, 0 });
		if (rc.IsPointInside(cursorPos))
			return eWindowCaptionButton::MINIMIZE;
		if (cursorPos.y < 55) {
			return eWindowCaptionButton::BAR;
		}
		return eWindowCaptionButton::NONE;
	};
	//window.SetBorderless(true);
	//window.SetCaptionButtonHandler(CaptionHandler);

	while (!window.IsClosed()) {
		inputHandler->SetFocused(window.IsFocused());
		window.CallEvents();
		if (joyInput) {
			joyInput->CallEvents();
		}
		if (keyboardInput) {
			keyboardInput->CallEvents();
		}

		try {
			// Update world
			qcWorld->UpdateWorld((float)frameTime);
			qcWorld->RenderWorld((float)frameTime);

			// Calculate elapsed time for frame.
			frameTime = timer.Elapsed();
			timer.Reset();

			// Calculate average framerate
			frameRateUpdate += frameTime;
			if (frameRateUpdate > 0.5) {
				frameRateUpdate = 0;

				double avgFrameTime = 0.0;
				for (auto v : frameTimeHistory) {
					avgFrameTime += v;
				}
				avgFrameTime /= frameTimeHistory.size();
				avgFps = 1.0f / (float)avgFrameTime;

				frameTimeHistory.clear();
			}
			frameTimeHistory.push_back(frameTime);

			// Set info text as window title
			unsigned width, height;
			engine->GetScreenSize(width, height);
			std::string title = "Graphics Engine Test | " + std::to_string(width) + "x" + std::to_string(height) + " | FPS=" + std::to_string((int)avgFps);
			window.SetTitle(title);
		}
		catch (Exception& ex) {
			std::stringstream trace;
			trace << "Graphics engine error:" << ex.what() << "\n";
			ex.PrintStackTrace(trace);
			systemLogStream.Event(trace.str());
			PostQuitMessage(0);
		}
		catch (std::exception& ex) {
			systemLogStream.Event(std::string("Graphics engine error: ") + ex.what());
			logger.Flush();
			PostQuitMessage(0);
		}
	}

	cout << "Shutting down." << endl;
	return 0;
}


void InputHandler::OnJoystickMove(JoystickMoveEvent evt) {
	if (!world || !m_focused) {
		return;
	}
	cout << "Axis" << evt.axis << " = " << evt.absPos << endl;
	switch (evt.axis) {
		case 0: world->TiltForward(-evt.absPos); break;
		case 1: world->TiltRight(evt.absPos); break;
		case 2: world->Ascend(-std::copysign(1.0f, evt.absPos)*std::max(0.0f, std::abs(evt.absPos)-0.15f)); break;
		case 3: world->RotateRight(evt.absPos); break;
		default:
			break;
	}
}


void InputHandler::OnKey(KeyboardEvent evt) {
	if (!world || !m_focused) {
		return;
	}
	float enable = float(evt.state == eKeyState::DOWN);
	switch (evt.key) {
		case eKey::W: world->TiltForward(enable); break;
		case eKey::A: world->TiltLeft(enable); break;
		case eKey::S: world->TiltBackward(enable); break;
		case eKey::D: world->TiltRight(enable); break;
		case eKey::LEFT: world->RotateLeft(enable); break;
		case eKey::RIGHT: world->RotateRight(enable); break;
		case eKey::UP: world->Ascend(enable); break;
		case eKey::DOWN: world->Descend(enable); break;
		default:
			break;
	}
}

void InputHandler::OnResize(ResizeEvent evt) {
	if (world) {
		world->ScreenSizeChanged(evt.clientSize.x, evt.clientSize.y);
	}
}


std::string SelectPipeline(IGraphicsApi* gxapi) {
	std::unique_ptr<ICapabilityQuery> query(gxapi->GetCapabilityQuery());

	auto binding = query->QueryResourceBinding();
	auto tiled = query->QueryTiledResources();
	auto conserv = query->QueryConservativeRasterization();
	auto heaps = query->QueryResourceHeaps();
	auto additional = query->QueryAdditional();

	int bindingTier = binding.GetDx12Tier();
	int tiledTier = tiled.GetDx12Tier();
	int conservTier = conserv.GetDx12Tier();
	int heapsTier = heaps.GetDx12Tier();

	int maxVmemResourceGB = (1ull << additional.virtualAddressBitsPerResource) / 1024/1024/1024;
	int maxVmemProcessGB = (1ull << additional.virtualAddressBitsPerProcess) / 1024/1024/1024;

	cout << "Selected GPU supports:" << endl;
	cout << "   Resource binding:      Tier" << bindingTier << endl;
	cout << "   Tiled resources:       Tier" << tiledTier << endl;
	cout << "   Conservative raster.:  " << (conservTier == 0 ? std::string("UNSUPPORTED") : "Tier" + std::to_string(conservTier)) << endl;
	cout << "   Resource heaps:        Tier" << heapsTier << endl;
	cout << "   Process virtual memory: " << maxVmemProcessGB << " GiB, resource vmem: " << maxVmemResourceGB << " GiB" << endl;
	cout << "   Shader model " << additional.shaderModelMajor << "." << additional.shaderModelMinor << endl;

	CapsRequirementSet giReqs;
	giReqs.conservativeRasterization = CapsConservativeRasterization::Dx12Tier1();
	giReqs.formats.push_back({ eFormat::R32G32B32A32_FLOAT, eCapsFormatUsage({eCapsFormatUsage::UNORDERED_ACCESS_LOAD, eCapsFormatUsage::UNORDERED_ACCESS_STORE, eCapsFormatUsage::TEXTURE_3D}) });

	bool supportsGi = query->SupportsAll(giReqs);
	if (supportsGi) {
		return "pipeline.json";
	}
	else {
		return "pipeline_nogi.json";
	}
	
}


void OnTerminate() {
	try {
		std::rethrow_exception(std::current_exception());
		systemLogStream.Event(std::string("Terminate called, shutting down services."));
	}
	catch (Exception& ex) {
		systemLogStream.Event(std::string("Terminate called, shutting down services.") + ex.what() + "\n" + ex.StackTraceStr());
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
}