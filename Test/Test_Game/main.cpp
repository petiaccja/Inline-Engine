#include <BaseLibrary/Platform/System.hpp>
#include <BaseLibrary/Platform/Window.hpp>
#include <BaseLibrary/Timer.hpp>
#include <GameLogic/World.hpp>
#include <GraphicsApi_D3D12/GxapiManager.hpp>
#include <GraphicsEngine_LL/GraphicsEngine.hpp>
#include <GraphicsEngine_LL/DirectionalLight.hpp>

#include <GameFoundationLibrary/Systems/GraphicsTransformSystem.hpp>
#include <GameFoundationLibrary/Systems/LinkTransformSystem.hpp>
#include <GameFoundationLibrary/Systems/RenderingSystem.hpp>

#include "DebugInfoFrame.hpp"
#include "UserInterface.hpp"


using namespace inl;


struct Modules {
	std::unique_ptr<gxapi::IGxapiManager> gxapiManager;
	std::unique_ptr<gxapi::IGraphicsApi> graphicsApi;
	std::unique_ptr<gxeng::IGraphicsEngine> graphicsEngine;
	gxapi::AdapterInfo info;
};


struct Systems {
	gamelib::RenderingSystem renderingSystem;
	gamelib::LinkTransformSystem linkTransformSystem;
};


struct RenderScene {
	std::unique_ptr<gxeng::IScene> mainScene;
	std::unique_ptr<gxeng::IPerspectiveCamera> mainCamera;
	std::unique_ptr<gxeng::IScene> guiScene;
	std::unique_ptr<gxeng::ICamera2D> guiCamera;
};


Modules CreateModules(Window& window, Logger& logger) {
	std::unique_ptr<gxapi::IGxapiManager> gxapiManager = std::make_unique<gxapi_dx12::GxapiManager>();

	// Get first hardware adapter, if none, get software, if none, quit.
	auto graphicsAdapters = gxapiManager->EnumerateAdapters();
	int graphicsAdapterId = -1;
	gxapi::AdapterInfo info;
	for (auto& adapter : graphicsAdapters) {
		graphicsAdapterId = adapter.adapterId;
		info = adapter;
		if (!adapter.isSoftwareAdapter) {
			break;
		}
	}
	if (graphicsAdapterId == -1) {
		throw RuntimeException("Could not find suitable graphics card or software rendering driver for DirectX 12.");
	}

	std::unique_ptr<gxapi::IGraphicsApi> graphicsApi(gxapiManager->CreateGraphicsApi(graphicsAdapterId));

	gxeng::GraphicsEngineDesc graphicsEngineDesc{
		gxapiManager.get(),
		graphicsApi.get(),
		window.GetNativeHandle(),
		false,
		(int)window.GetClientSize().x,
		(int)window.GetClientSize().y,
		&logger
	};
	std::unique_ptr<gxeng::IGraphicsEngine> graphicsEngine = std::make_unique<gxeng::GraphicsEngine>(graphicsEngineDesc);

	return { std::move(gxapiManager), std::move(graphicsApi), std::move(graphicsEngine), info };
}


Systems CreateSystems(const Modules& modules) {
	gamelib::RenderingSystem renderingSystem{ modules.graphicsEngine.get() };
	gamelib::LinkTransformSystem linkTransformSystem;

	return { std::move(renderingSystem), std::move(linkTransformSystem) };
}


void SetupGraphicsEngine(gxeng::IGraphicsEngine* graphicsEngine) {
	graphicsEngine->SetShaderDirectories({ INL_NODE_SHADER_DIRECTORY,
										   INL_MTL_SHADER_DIRECTORY,
										   "./Shaders",
										   "./Materials" });

	std::ifstream pipelineFile(INL_GAMEDATA "/Pipelines/new_forward_with_gui.json");
	if (!pipelineFile.is_open()) {
		throw FileNotFoundException("Failed to open pipeline JSON.");
	}
	std::string pipelineDesc((std::istreambuf_iterator<char>(pipelineFile)), std::istreambuf_iterator<char>());
	graphicsEngine->LoadPipeline(pipelineDesc);

}


RenderScene CreateRenderScene(gxeng::IGraphicsEngine* graphicsEngine) {
	RenderScene s;
	s.mainScene.reset(graphicsEngine->CreateScene("MainScene"));
	s.mainCamera.reset(graphicsEngine->CreatePerspectiveCamera("MainCamera"));
	s.guiScene.reset(graphicsEngine->CreateScene("GuiScene"));
	s.guiCamera.reset(graphicsEngine->CreateCamera2D("GuiCamera"));
	return s;
}


int main() {
	try {
		Timer timer;
		Logger logger;
		Window window("Test game");

		Modules modules = CreateModules(window, logger);

		SetupGraphicsEngine(modules.graphicsEngine.get());

		game::World gameWorld;
		Systems systems = CreateSystems(modules);

		gameWorld.SetSystems({ &systems.linkTransformSystem,
							   &systems.renderingSystem });

		RenderScene renderScene = CreateRenderScene(modules.graphicsEngine.get());

		std::unique_ptr<gxeng::DirectionalLight> sunLight = std::make_unique<gxeng::DirectionalLight>();
		sunLight->SetDirection(Vec3{-0.2f, -0.3f, -0.6f}.Normalized());
		sunLight->SetColor({ 0.9f, 0.8f, 0.7f });
		renderScene.mainScene->GetEntities<gxeng::DirectionalLight>().Add(sunLight.get());

		UserInterface uiHost(*modules.graphicsEngine, *renderScene.guiScene, *renderScene.guiCamera);
		DebugInfoFrame debugInfoFrame;
		debugInfoFrame.SetAdapterInfo(modules.info);
		debugInfoFrame.SetResolutionInfo(window.GetClientSize());
		uiHost.AddFrame(debugInfoFrame);
		debugInfoFrame.SetSize({ 500, 100 });

		renderScene.mainCamera->SetPosition({ 0, 0, 0.2 });
		renderScene.mainCamera->SetTarget({ 0, 1, 0 });
		renderScene.mainCamera->SetNearPlane(0.2f);
		renderScene.mainCamera->SetFarPlane(4000.f);

		window.OnResize += [&](ResizeEvent evt) {
			modules.graphicsEngine->SetScreenSize(evt.clientSize.x, evt.clientSize.y);
			debugInfoFrame.SetResolutionInfo(evt.clientSize);
			uiHost.SetResolution(window.GetClientSize(), window.GetClientSize());
			uiHost[debugInfoFrame].SetPosition(Vec2(0, evt.clientSize.y) + Vec2(0.5f, -0.5f) * debugInfoFrame.GetSize());
			renderScene.mainCamera->SetFOVAspect(Deg2Rad(70.f), float(evt.clientSize.x) / float(evt.clientSize.y));
		};
		window.OnResize(ResizeEvent{ window.GetSize(), window.GetClientSize(), eResizeMode::RESTORED });

		timer.Start();
		while (!window.IsClosed()) {
			float frameTime = float(timer.Elapsed());
			timer.Reset();
			gameWorld.Update(frameTime);
			uiHost.Update(frameTime);
			window.CallEvents();
		}
	}
	catch (std::exception& ex) {
		std::cout << "Error: " << ex.what() << std::endl;
	}
}
