#include "Core.hpp"

Core::Core()
:guiEngine(nullptr), graphicsEngine(nullptr), graphicsApi(nullptr), graphicsApiMgr(nullptr)
{

}

Core::~Core()
{
	delete guiEngine;
	delete graphicsEngine;
	delete graphicsApi;
	delete graphicsApiMgr;
}

GraphicsEngine* Core::InitGraphicsEngine(int width, int height, HWND hwnd)
{
	if (graphicsEngine)
	{
		delete graphicsEngine;
		delete graphicsApi;
		delete graphicsApiMgr;
	}

	// Create Graphics Api
	GxapiManager* graphicsApiMgr = new GxapiManager();
	auto adapters = graphicsApiMgr->EnumerateAdapters();
	graphicsApi = graphicsApiMgr->CreateGraphicsApi(adapters[0].adapterId);
	
	// Create GraphicsEngine
	GraphicsEngineDesc desc;
	desc.fullScreen = false;
	desc.graphicsApi = graphicsApi;
	desc.gxapiManager = graphicsApiMgr;
	desc.width = width;
	desc.height = height;
	desc.targetWindow = hwnd;
	desc.logger = &logger;
	
	graphicsEngine = new GraphicsEngine(desc);

	return graphicsEngine;
}

GuiEngine* Core::InitGuiEngine(GraphicsEngine* graphicsEngine, Window* targetWindow)
{
	if (guiEngine)
		delete guiEngine;

	guiEngine = new GuiEngine(graphicsEngine, targetWindow);
	return guiEngine;
}

void Core::Update(float deltaTime)
{
	graphicsEngine->Update(deltaTime);
	guiEngine->Update(deltaTime);
}