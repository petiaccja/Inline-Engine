#pragma once
#include "GuiText.hpp"
#include "GuiButton.hpp"
#include "GuiList.hpp"
#include "GuiSlider.hpp"
#include "GuiCollapsable.hpp"
#include <vector>

namespace inl::gui {

class GuiLayer : public Gui
{
public:
	GuiLayer(GuiEngine* guiEngine);

	virtual Vector2f MeasureChildren(const Vector2f& availableSize);
	virtual Vector2f ArrangeChildren(const Vector2f& finalSize);

protected:
	GuiEngine* guiEngine;
};

inline Vector2f GuiLayer::MeasureChildren(const Vector2f& availableSize)
{
	for (Gui* child : GetChildren())
		child->Measure(child->GetSize());

	return GetSize();
}

inline Vector2f GuiLayer::ArrangeChildren(const Vector2f& finalSize)
{
	for (Gui* child : GetChildren())
		child->Arrange(child->GetPos(), child->desiredSize);

	return GetSize();
}

//inline GuiLayer::~GuiLayer()
//{
//	delete layer;
//}
//
//inline void GuiLayer::Add(Gui* child)
//{
//	layer->Add(child);
//}
//
//inline bool GuiLayer::Remove(Gui* child)
//{
//	return layer->Remove(child);
//}
//
//
//template<class T>
//inline T* GuiLayer::AddControl()
//{
//	return layer->Add<T>();
//}
//
//inline Gui* GuiLayer::AddGui()
//{
//	return AddControl<Gui>();
//}
//
//inline GuiButton* GuiLayer::AddButton()
//{
//	return AddControl<GuiButton>();
//}
//
//inline GuiText* GuiLayer::AddText()
//{
//	return AddControl<GuiText>();
//}
//
//inline GuiList* GuiLayer::AddList()
//{
//	return AddControl<GuiList>();
//}
//
//inline Gui* GuiLayer::AddPlane()
//{
//	return AddControl<Gui>();
//}
//
//inline GuiSlider* GuiLayer::AddSlider()
//{
//	return AddControl<GuiSlider>();
//}
//
//inline GuiCollapsable* GuiLayer::AddCollapsable()
//{
//	return AddControl<GuiCollapsable>();
//}
//
//inline const std::vector<Gui*>& GuiLayer::GetControls() const
//{
//	return layer->GetChildren();
//}

} // namespace inl::gui