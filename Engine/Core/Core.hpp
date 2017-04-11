#pragma once
#include <GuiEngine/GuiEngine.hpp>
#include <BaseLibrary/Platform/Window.hpp>
#include <GraphicsEngine_LL/GraphicsEngine.hpp>
using namespace inl::gxeng;

class EngineCore
{
public:
	EngineCore();
	~EngineCore();

	GraphicsEngine* InitGraphicsEngine(int width, int height, HWND hwnd);
	GuiEngine* InitGuiEngine(GraphicsEngine* graphicsEngine, Window* targetWindow);

	void Update(float deltaTime);

protected:
	GraphicsEngine* graphicsEngine;
	GuiEngine* guiEngine;
	exc::Logger logger;
};