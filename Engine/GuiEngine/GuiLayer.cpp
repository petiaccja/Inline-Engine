#pragma once
#include "GuiLayer.hpp"
#include "GuiEngine.hpp"

using namespace inl::gui;

GuiLayer::GuiLayer(GuiEngine* guiEngine)
:Gui(guiEngine, true)
{
	Vector2u windowContentArea = guiEngine->GetTargetWindow()->GetClientSize();
	SetSize(Vector2f(windowContentArea.x(), windowContentArea.y()));

	HideBgColor();
	SetName("layer");
}

Vector2f GuiLayer::ArrangeChildren(const Vector2f& finalSize)
{
	for (Gui* child : GetChildren())
		child->Arrange(child->GetPos(), child->GetDesiredSize());

	return GetSize();
}