#pragma once
#include <GraphicsEngine/IGraphicsEngine.hpp>
#include <GuiEngine/GuiEngine.hpp>
using namespace inl::gxeng;

class EngineCore
{
public:
	IGraphicsEngine* InitGraphicsEngine(int width, int height, HWND hwnd);
	GuiEngine* InitGuiEngine(IGraphicsEngine* graphicsEngine);

	void Update(float deltaTime);

protected:
	IGraphicsEngine* graphicsEngine;
	GuiEngine* guiEngine;
};