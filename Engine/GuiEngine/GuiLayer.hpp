#pragma once
#include "GuiControl.hpp"
#include <vector>

class GuiLayer
{
public:
	~GuiLayer();

	GuiControl* AddControl();

protected:
	std::vector<GuiControl*> controls;
};

inline GuiLayer::~GuiLayer()
{
	for(auto& a : controls)
		delete a;

	controls.clear();
}

inline GuiControl* GuiLayer::AddControl()
{
	GuiControl* ctrl = new GuiControl();
	controls.push_back(ctrl);
	return ctrl;
}