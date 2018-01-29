#pragma once
#include "Layer.hpp"
#include "GuiEngine.hpp"

using namespace inl::gui;

Layer::Layer(GuiEngine& guiEngine)
:Gui(guiEngine, true)
{
	Vec2u windowContentArea = guiEngine.GetTargetWindow()->GetClientSize();
	SetSize(Vec2(windowContentArea.x, windowContentArea.y));

	HideBgColor();
	SetName("layer");
}

Vec2 Layer::ArrangeChildren()
{
	for (Gui* child : GetChildren())
		child->Arrange(child->GetPos(), child->GetDesiredSize());

	return GetSize();
}