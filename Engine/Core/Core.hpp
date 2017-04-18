#pragma once
#include <GuiEngine/GuiEngine.hpp>
#include <GraphicsEngine_LL/GraphicsEngine.hpp>
#include <GraphicsApi_D3D12/GxapiManager.hpp>
#include <GraphicsEngine/Definitions.hpp>
#include <GraphicsEngine_LL/GraphicsEngine.hpp>

#include <BaseLibrary/Platform/Window.hpp>

using namespace inl::gxeng;
using namespace inl::gxapi_dx12;
using namespace inl::gxapi;
using namespace inl::gui;

class Core
{
public:
	Core();
	~Core();

	GraphicsEngine* InitGraphicsEngine(int width, int height, HWND hwnd);
	GuiEngine* InitGuiEngine(GraphicsEngine* graphicsEngine, Window* targetWindow);

	void Update(float deltaTime);

protected:
	GraphicsEngine* graphicsEngine;
	GxapiManager* graphicsApiMgr;
	IGraphicsApi* graphicsApi;

	GuiEngine* guiEngine;
	exc::Logger logger;
};