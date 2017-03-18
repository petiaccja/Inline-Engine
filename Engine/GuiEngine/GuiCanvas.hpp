#pragma once
#include "GuiLayer.hpp"

class GuiCanvas
{
public:
	~GuiCanvas();

	GuiLayer* AddLayer();
	const std::vector<GuiLayer*>& GetLayers() const;

protected:
	std::vector<GuiLayer*> layers;
};

inline GuiCanvas::~GuiCanvas()
{
	for(auto& layer : layers)
		delete layer;

	layers.clear();
}

inline GuiLayer* GuiCanvas::AddLayer()
{
	GuiLayer* layer = new GuiLayer();
	layers.push_back(layer);
	return layer;
}

inline const std::vector<GuiLayer*>& GuiCanvas::GetLayers() const
{
	return layers;
}