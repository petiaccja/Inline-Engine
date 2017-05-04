#pragma once
#include <GuiEngine/GuiEngine.hpp>
#include <GraphicsEngine_LL/GraphicsEngine.hpp>
#include <GraphicsApi_D3D12/GxapiManager.hpp>
#include <GraphicsEngine/Definitions.hpp>
#include <GraphicsEngine_LL/GraphicsEngine.hpp>

#include <BaseLibrary/Platform/Window.hpp>

using namespace inl;
using namespace inl::gui;

class Core
{
public:
	Core();
	~Core();

	gxeng::GraphicsEngine* InitGraphicsEngine(int width, int height, HWND hwnd);
	GuiEngine* InitGuiEngine(gxeng::GraphicsEngine* graphicsEngine, Window* targetWindow);

	void Update(float deltaTime);

protected:
	gxeng::GraphicsEngine* graphicsEngine;
	gxapi_dx12::GxapiManager* graphicsApiMgr;
	gxapi::IGraphicsApi* graphicsApi;

	GuiEngine* guiEngine;
	exc::Logger logger;
};