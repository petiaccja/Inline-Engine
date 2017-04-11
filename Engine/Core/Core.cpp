#include "Core.hpp"
#include <GraphicsApi_D3D12\GxapiManager.hpp>
#include <GraphicsEngine\Definitions.hpp>
#include <GraphicsEngine_LL\GraphicsEngine.hpp>

using namespace inl::gxapi_dx12;
using namespace inl::gxapi;
using namespace inl::gxeng;

EngineCore::EngineCore()
:guiEngine(nullptr), graphicsEngine(nullptr)
{

}

EngineCore::~EngineCore()
{
	delete guiEngine;
	delete graphicsEngine;
}

GraphicsEngine* EngineCore::InitGraphicsEngine(int width, int height, HWND hwnd)
{
	// Create Graphics Api
	GxapiManager* gxApiMgr = new GxapiManager();
	auto adapters = gxApiMgr->EnumerateAdapters();
	IGraphicsApi* gxApi = gxApiMgr->CreateGraphicsApi(adapters[0].adapterId);
	
	// Create GraphicsEngine
	GraphicsEngineDesc desc;
	desc.fullScreen = false;
	desc.graphicsApi = gxApi;
	desc.gxapiManager = gxApiMgr;
	desc.width = width;
	desc.height = height;
	desc.targetWindow = hwnd;
	desc.logger = &logger;
	
	graphicsEngine = new GraphicsEngine(desc);

	return graphicsEngine;
}

GuiEngine* EngineCore::InitGuiEngine(GraphicsEngine* graphicsEngine, Window* targetWindow)
{
	if (guiEngine)
		delete guiEngine;

	guiEngine = new GuiEngine(graphicsEngine, targetWindow);
	return guiEngine;
}

void EngineCore::Update(float deltaTime)
{
	graphicsEngine->Update(deltaTime);
	guiEngine->Update(deltaTime);
}