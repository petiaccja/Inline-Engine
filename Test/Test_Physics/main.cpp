#include <iostream>

#include <BaseLibrary/Logging_All.hpp>
#include <BaseLibrary/Platform/Window.hpp>
#include <GraphicsApi_D3D12/GxapiManager.hpp>
#include <GraphicsApi_LL/IGxapiManager.hpp>
#include <GraphicsApi_LL/IGraphicsApi.hpp>
#include <GraphicsEngine_LL/GraphicsEngine.hpp>

#include <GraphicsEngine_LL/Scene.hpp>
#include <GraphicsEngine_LL/PerspectiveCamera.hpp>
#include <GraphicsEngine_LL/DirectionalLight.hpp>


using namespace inl;


class GameScene {
public:
	GameScene(gxeng::GraphicsEngine* graphicsEngine) : m_graphicsEngine(graphicsEngine) {
		m_gxScene.reset(m_graphicsEngine->CreateScene("World"));
		m_camera.reset(m_graphicsEngine->CreatePerspectiveCamera("WorldCam"));
		m_light.reset(new gxeng::DirectionalLight());

		m_light->SetDirection(Vec3{ -1, -2, -3 }.Normalized());
		m_light->SetColor({ 0.7f, 0.8f, 0.9f });

		m_gxScene->GetEntities<gxeng::DirectionalLight>().Add(m_light.get());

		m_camera->SetTargeted(true);
		m_camera->SetPosition({ 5, 5, 5 });
		m_camera->SetTarget({ 0,0,0 });
		m_camera->SetUpVector({ 0,0,1 });
	}
private:
	gxeng::GraphicsEngine* m_graphicsEngine;
	std::unique_ptr<gxeng::Scene> m_gxScene;
	std::unique_ptr<gxeng::PerspectiveCamera> m_camera;

	std::unique_ptr<gxeng::DirectionalLight> m_light;
};


int main() {
	try {
		// Create logger and window.
		Logger logger;
		Window window{ "Physics test", {1024, 640}, false, false };

		// Create graphics API.
		std::unique_ptr<gxapi::IGxapiManager> gxapiManager(new gxapi_dx12::GxapiManager());

		auto adapters = gxapiManager->EnumerateAdapters();
		if (adapters.empty()) {
			throw RuntimeException("No suitable graphics adapter found.");
		}

		std::unique_ptr<gxapi::IGraphicsApi> graphicsApi(gxapiManager->CreateGraphicsApi(adapters[0].adapterId));


		// Create graphics engine.
		gxeng::GraphicsEngineDesc graphicsEngineDesc;
		graphicsEngineDesc.gxapiManager = gxapiManager.get();
		graphicsEngineDesc.graphicsApi = graphicsApi.get();
		graphicsEngineDesc.fullScreen = false;
		graphicsEngineDesc.width = window.GetClientSize().x;
		graphicsEngineDesc.height = window.GetClientSize().y;
		graphicsEngineDesc.logger = &logger;
		graphicsEngineDesc.targetWindow = window.GetNativeHandle();
		std::unique_ptr<gxeng::GraphicsEngine> graphicsEngine(new gxeng::GraphicsEngine(graphicsEngineDesc));

		// Load graphics pipeline.
		std::ifstream pipelineFile(INL_PIPELINE_DIRECTORY "\\pipeline_noeffect.json");
		if (!pipelineFile.is_open()) {
			throw FileNotFoundException("Failed to open pipeline JSON.");
		}
		std::string pipelineDesc((std::istreambuf_iterator<char>(pipelineFile)), std::istreambuf_iterator<char>());
		graphicsEngine->LoadPipeline(pipelineDesc);
		graphicsEngine->SetShaderDirectories({ INL_NODE_SHADER_DIRECTORY, INL_MTL_SHADER_DIRECTORY, "./Shaders", "./Materials" });

		// Create scene and camera.
		GameScene gameScene(graphicsEngine.get());

		// Game loop.
		while (!window.IsClosed()) {
			window.CallEvents();
			graphicsEngine->Update(0.016f);
		}

		return 0;
	}
	catch (Exception& ex) {
		using namespace std;
		cout << "Unhandled exception occured." << endl;
		cout << "MESSAGE:" << ex.what() << endl;
		cout << "STACK TRACE:" << endl;
		ex.PrintStackTrace(cout);
		cout << "PRESS ENTER TO QUIT..." << endl;
		cin.get();
	}
}