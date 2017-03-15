#include "Core.hpp"
#include <GraphicsApi_D3D12\GxapiManager.hpp>
#include <GraphicsEngine\Definitions.hpp>
#include <GraphicsEngine_LL\GraphicsEngine.hpp>

using namespace inl::gxapi_dx12;
using namespace inl::gxapi;
using namespace inl::gxeng;

EngineCore Core;

IGraphicsEngine* EngineCore::InitGraphicsEngine()
{
	//// Create manager
	//GxapiManager* gxApiMgr = new GxapiManager();
	//auto adapters = gxApiMgr->EnumerateAdapters();
	//
	//auto adapters = gxApiMgr->EnumerateAdapters();
	//
	//// Create graphics api
	//IGraphicsApi* gxApi = gxApiMgr->CreateGraphicsApi(adapters[0].adapterId);
	//
	////GraphicsEngineDesc desc;
	////desc.fullScreen = false;
	////desc.graphicsApi = gxApi;
	////desc.gxapiManager = gxApiMgr;
	////desc.width = width;
	////desc.height = height;
	////desc.targetWindow = hWnd;
	////desc.logger = &logger;
	////
	////engine.reset(new GraphicsEngine(desc));
	////pEngine = engine.get();
	return nullptr;
}

GuiEngine* EngineCore::InitGuiEngine(IGraphicsEngine * graphicsEngine)
{
	if (guiEngine)
		delete guiEngine;

	guiEngine = new GuiEngine(graphicsEngine);
	return guiEngine;
}

void EngineCore::Update(float DeltaTime)
{

}