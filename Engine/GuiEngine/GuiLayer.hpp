#pragma once
#include "GuiText.hpp"
#include "GuiButton.hpp"
#include "GuiList.hpp"
#include "GuiSlider.hpp"
#include <vector>

class GuiLayer
{
public:
	GuiLayer(GuiEngine* guiEngine);
	~GuiLayer();

	void AddControl(GuiControl* child);
	bool RemoveControl(GuiControl* child);

	GuiControl* AddControl();
	GuiButton* AddButton();
	GuiText* AddText();
	GuiList* AddList();
	GuiPlane* AddPlane();
	GuiSlider* AddSlider();

	const std::vector<GuiControl*>& GetControls() const;

protected:
	template<class T>
	T* AddControl();

protected:
	GuiControl* layer;

	GuiEngine* guiEngine;
};

inline GuiLayer::GuiLayer(GuiEngine* guiEngine)
:guiEngine(guiEngine), layer(new GuiControl(guiEngine))
{
}

inline GuiLayer::~GuiLayer()
{
	delete layer;
}

inline void GuiLayer::AddControl(GuiControl* child)
{
	layer->Add(child);
}

inline bool GuiLayer::RemoveControl(GuiControl* child)
{
	return layer->RemoveChild(child);
}


template<class T>
inline T* GuiLayer::AddControl()
{
	return layer->Add<T>();
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

inline GuiPlane* GuiLayer::AddPlane()
{
	return AddControl<GuiPlane>();
}

inline GuiSlider* GuiLayer::AddSlider()
{
	return AddControl<GuiSlider>();
}

inline const std::vector<GuiControl*>& GuiLayer::GetControls() const
{
	return layer->GetChildren();
}