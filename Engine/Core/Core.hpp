#pragma once
#include <GraphicsEngine/IGraphicsEngine.hpp>
#include <GuiEngine/GuiEngine.hpp>
#include <BaseLibrary\Platform\Window.hpp>
using namespace inl::gxeng;

class EngineCore
{
public:
	EngineCore();

	IGraphicsEngine* InitGraphicsEngine(int width, int height, HWND hwnd);
	GuiEngine* InitGuiEngine(IGraphicsEngine* graphicsEngine, Window* targetWindow);

	void Update(float deltaTime);

protected:
	IGraphicsEngine* graphicsEngine;
	GuiEngine* guiEngine;
};