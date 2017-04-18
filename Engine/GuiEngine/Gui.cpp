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

GuiCollapsable* Gui::AddCollapsable()
{
	return Add<GuiCollapsable>();
}

float Gui::GetContentSpaceCursorPosX()
{
	return guiEngine->GetWindowCursorPosX() - pos.x();
}

float Gui::GetContentSpaceCursorPosY()
{
	return guiEngine->GetWindowCursorPosY() - pos.y();
}