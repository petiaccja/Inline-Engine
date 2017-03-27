#pragma once
#include "GuiControl.hpp"
#include "GuiPlane.hpp"
#include "GuiText.hpp"
#include "GuiButton.hpp"
#include "GuiList.hpp"
#include "GuiEngine.hpp"

GuiPlane* GuiControl::AddPlane()
{
	return Add<GuiPlane>();
}

GuiText* GuiControl::AddText()
{
	return Add<GuiText>();
}

GuiButton* GuiControl::AddButton()
{
	return Add<GuiButton>();
}

GuiList* GuiControl::AddList()
{
	return Add<GuiList>();
}

float GuiControl::GetClientCursorPosX()
{
	return guiEngine->GetWindowCursorPosX() - pos.x;
}

float GuiControl::GetClientCursorPosY()
{
	return guiEngine->GetWindowCursorPosY() - pos.y;
}