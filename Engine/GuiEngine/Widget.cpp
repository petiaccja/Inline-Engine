#pragma once
#include "Widget.hpp"
#include "GuiText.hpp"
#include "GuiButton.hpp"
#include "GuiList.hpp"
#include "GuiEngine.hpp"

using namespace inl::gui;

Widget* Widget::AddWidget()
{
	return Add<Widget>();
}

GuiText* Widget::AddText()
{
	return Add<GuiText>();
}

GuiButton* Widget::AddButton()
{
	return Add<GuiButton>();
}

GuiList* Widget::AddList()
{
	return Add<GuiList>();
}

float Widget::GetClientSpaceCursorPosX()
{
	return guiEngine->GetWindowCursorPosX() - pos.x;
}

float Widget::GetClientSpaceCursorPosY()
{
	return guiEngine->GetWindowCursorPosY() - pos.y;
}