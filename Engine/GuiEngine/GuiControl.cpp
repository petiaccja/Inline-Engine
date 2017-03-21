#pragma once
#include "GuiControl.hpp"
#include "GuiPlane.hpp"
#include "GuiText.hpp"
#include "GuiButton.hpp"

GuiPlane* GuiControl::AddPlane()
{
	return AddChild<GuiPlane>();
}

GuiText* GuiControl::AddText()
{
	return AddChild<GuiText>();
}

GuiButton* GuiControl::AddButton()
{
	return AddChild<GuiButton>();
}
