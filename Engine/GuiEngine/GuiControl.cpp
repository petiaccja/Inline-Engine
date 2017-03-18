#pragma once
#include "GuiControl.hpp"
#include "GuiPlane.hpp"
#include "GuiText.hpp"
#include "GuiButton.hpp"

GuiPlane* GuiControl::AddChildPlane()
{
	return AddChild<GuiPlane>();
}

GuiText* GuiControl::AddChildText()
{
	return AddChild<GuiText>();
}

GuiButton* GuiControl::AddChildButton()
{
	return AddChild<GuiButton>();
}
