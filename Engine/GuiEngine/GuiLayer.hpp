#pragma once
#include "GuiText.hpp"
#include "GuiButton.hpp"
#include "GuiList.hpp"
#include "GuiSlider.hpp"
#include "GuiCollapsable.hpp"
#include <vector>

class GuiLayer
{
public:
	GuiLayer(GuiEngine* guiEngine);
	~GuiLayer();

	void Add(Widget* child);
	bool Remove(Widget* child);

	Widget* AddControl();
	GuiButton* AddButton();
	GuiText* AddText();
	GuiList* AddList();
	Widget* AddPlane();
	GuiSlider* AddSlider();
	GuiCollapsable* AddCollapsable();

	const std::vector<Widget*>& GetControls() const;

protected:
	template<class T>
	T* AddControl();

protected:
	Widget* layer;

	GuiEngine* guiEngine;
};

inline GuiLayer::GuiLayer(GuiEngine* guiEngine)
:guiEngine(guiEngine), layer(new Widget(guiEngine, true))
{
}

inline GuiLayer::~GuiLayer()
{
	delete layer;
}

inline void GuiLayer::Add(Widget* child)
{
	layer->Add(child);
}

inline bool GuiLayer::Remove(Widget* child)
{
	return layer->Remove(child);
}


template<class T>
inline T* GuiLayer::AddControl()
{
	return layer->Add<T>();
}

inline Widget* GuiLayer::AddControl()
{
	return AddControl<Widget>();
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

inline Widget* GuiLayer::AddPlane()
{
	return AddControl<Widget>();
}

inline GuiSlider* GuiLayer::AddSlider()
{
	return AddControl<GuiSlider>();
}

inline GuiCollapsable* GuiLayer::AddCollapsable()
{
	return AddControl<GuiCollapsable>();
}

inline const std::vector<Widget*>& GuiLayer::GetControls() const
{
	return layer->GetChildren();
}