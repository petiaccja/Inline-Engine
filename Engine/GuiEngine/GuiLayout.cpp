#pragma once
#include "GuiLayout.hpp"
#include "GuiText.hpp"
#include "GuiButton.hpp"
#include "GuiList.hpp"
#include "GuiSlider.hpp"
#include "GuiCollapsable.hpp"
#include "GuiSplitter.hpp"
#include "GuiImage.hpp"

using namespace inl::gui;

GuiButton* GuiLayout::AddItemButton(const std::string& text /*= ""*/)
{
	GuiButton* btn = AddItem<GuiButton>();
	btn->SetText(text);
	return btn;
}

Gui* GuiLayout::AddItemSeparatorHor()
{
	Gui* btn = AddItem<Gui>();
	btn->SetSize(1, 1);
	btn->StretchHorFillParent();
	return btn;
}