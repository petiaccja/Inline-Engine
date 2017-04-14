#pragma once
#include "Gui.hpp"
#include "GuiText.hpp"
#include "GuiButton.hpp"
#include "GuiList.hpp"
#include "GuiEngine.hpp"

using namespace inl::gui;

Gui* Gui::AddGui()
{
	return Add<Gui>();
}

GuiText* Gui::AddText()
{
	return Add<GuiText>();
}

GuiButton* Gui::AddButton()
{
	return Add<GuiButton>();
}

GuiList* Gui::AddList()
{
	return Add<GuiList>();
}

GuiSlider* Gui::AddSlider()
{
	return Add<GuiSlider>();
}

float Gui::GetClientSpaceCursorPosX()
{
	return guiEngine->GetWindowCursorPosX() - pos.x();
}

float Gui::GetClientSpaceCursorPosY()
{
	return guiEngine->GetWindowCursorPosY() - pos.y();
}