#pragma once
#include "GuiCanvas.hpp"
#include <GraphicsEngine\IGraphicsEngine.hpp>
#include <vector>

using namespace inl::gxeng;

class GuiEngine
{
public:
	GuiEngine(IGraphicsEngine* graphicsEngine);
	~GuiEngine();

	GuiCanvas* AddCanvas();

	void Update(float deltaTime);

protected:
	IGraphicsEngine* graphicsEngine;

	std::vector<GuiCanvas*> canvases;
};

inline GuiEngine::GuiEngine(IGraphicsEngine* graphicsEngine)
:graphicsEngine(graphicsEngine)
{
}

inline GuiEngine::~GuiEngine()
{
	for(auto& canvas : canvases)
		delete canvas;

	canvases.clear();
}

inline GuiCanvas* GuiEngine::AddCanvas()
{
	GuiCanvas* canvas = new GuiCanvas();
	canvases.push_back(canvas);
	return canvas;
}

inline void GuiEngine::Update(float deltaTime)
{
	for(GuiCanvas* canvas : canvases)
	{

	}
}