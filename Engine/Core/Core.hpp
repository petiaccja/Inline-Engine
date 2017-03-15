#pragma once
#include <GraphicsEngine/IGraphicsEngine.hpp>
#include <GuiEngine/GuiEngine.hpp>
using namespace inl::gxeng;

class EngineCore
{
public:
	IGraphicsEngine* InitGraphicsEngine();
	GuiEngine* InitGuiEngine(IGraphicsEngine* graphicsEngine);

	void Update(float DeltaTime);

protected:
	IGraphicsEngine* graphicsEngine;
	GuiEngine* guiEngine;
};

extern EngineCore Core;