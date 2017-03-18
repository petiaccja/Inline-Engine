#pragma once
#include "GuiButton.hpp"
#include <vector>

class GuiLayer
{
public:
	~GuiLayer();

	GuiControl* AddControl();
	GuiButton* AddButton();

	const std::vector<GuiControl*>& GetControls() const;

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

inline GuiButton* GuiLayer::AddButton()
{
	GuiButton* ctrl = new GuiButton();
	controls.push_back(ctrl);
	return ctrl;
}

inline const std::vector<GuiControl*>& GuiLayer::GetControls() const
{
	return controls;
}