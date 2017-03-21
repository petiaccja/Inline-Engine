#pragma once
#include "GuiText.hpp"
#include "GuiButton.hpp"
#include "GuiList.hpp"
#include <vector>

class GuiLayer
{
public:
	~GuiLayer();

	GuiControl* AddControl();
	GuiButton* AddButton();
	GuiText* AddText();
	GuiList* AddList();

	const std::vector<GuiControl*>& GetControls() const;

protected:
	template<class T>
	T* AddControl();

protected:

	std::vector<GuiControl*> controls;
};

inline GuiLayer::~GuiLayer()
{
	for(auto& a : controls)
		delete a;

	controls.clear();
}

template<class T>
inline T* GuiLayer::AddControl()
{
	T* ctrl = new T();
	controls.push_back(ctrl);
	return ctrl;
}

inline GuiControl* GuiLayer::AddControl()
{
	return AddControl<GuiControl>();
}

inline GuiButton* GuiLayer::AddButton()
{
	return AddControl<GuiButton>();
}

inline GuiText* GuiLayer::AddText()
{
	return AddControl<GuiText>();
}

inline GuiList* GuiLayer::AddList()
{
	return AddControl<GuiList>();
}

inline const std::vector<GuiControl*>& GuiLayer::GetControls() const
{
	return controls;
}