#pragma once
#include "GuiLayouted.hpp"
#include "GuiText.hpp"
#include "GuiButton.hpp"
#include "GuiList.hpp"
#include "GuiSlider.hpp"
#include "GuiCollapsable.hpp"
#include "GuiSplitter.hpp"
#include "GuiImage.hpp"

using namespace inl::gui;

Gui* GuiLayouted::AddItemGui()
{
	return AddItem<Gui>();
}

GuiText* GuiLayouted::AddItemText()
{
	return AddItem<GuiText>();
}

GuiButton* GuiLayouted::AddItemButton(const std::string& text /*= ""*/)
{
	GuiButton* btn = AddItem<GuiButton>();
	btn->SetText(text);
	return btn;
}

GuiList* GuiLayouted::AddItemList()
{
	return AddItem<GuiList>();
}

GuiSlider* GuiLayouted::AddItemSlider()
{
	return AddItem<GuiSlider>();
}

GuiCollapsable* GuiLayouted::AddItemCollapsable()
{
	return AddItem<GuiCollapsable>();
}

GuiSplitter* GuiLayouted::AddItemSplitter()
{
	return AddItem<GuiSplitter>();
}

GuiImage* GuiLayouted::AddItemImage()
{
	return AddItem<GuiImage>();
}

Gui* GuiLayouted::AddItemSeparatorHor()
{
	Gui* btn = AddItemGui();
	btn->SetSize(1, 1);
	btn->StretchHorFillParent();
	return btn;
}