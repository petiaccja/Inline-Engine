#pragma once
#include "GuiLayer.hpp"
#include "GuiEngine.hpp"

using namespace inl::gui;

GuiLayer::GuiLayer(GuiEngine* guiEngine)
:Gui(guiEngine, true)
{
	Vector2u windowClientArea = guiEngine->GetTargetWindow()->GetClientSize();
	SetSize(Vector2f(windowClientArea.x(), windowClientArea.y()));

	SetBorder(3, Color::RED);
	HideBgColor();
}