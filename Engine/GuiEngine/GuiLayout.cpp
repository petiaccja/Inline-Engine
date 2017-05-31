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

Gui* GuiLayout::AddItemGui()
{
	return AddItem<Gui>();
}

GuiText* GuiLayout::AddItemText()
{
	return AddItem<GuiText>();
}

GuiButton* GuiLayout::AddItemButton(const std::string& text /*= ""*/)
{
	GuiButton* btn = AddItem<GuiButton>();
	btn->SetText(text);
	return btn;
}

GuiList* GuiLayout::AddItemList()
{
	return AddItem<GuiList>();
}

GuiSlider* GuiLayout::AddItemSlider()
{
	return AddItem<GuiSlider>();
}

GuiCollapsable* GuiLayout::AddItemCollapsable()
{
	return AddItem<GuiCollapsable>();
}

GuiSplitter* GuiLayout::AddItemSplitter()
{
	return AddItem<GuiSplitter>();
}

GuiImage* GuiLayout::AddItemImage()
{
	return AddItem<GuiImage>();
}

Gui* GuiLayout::AddItemSeparatorHor()
{
	Gui* btn = AddItemGui();
	btn->SetSize(1, 1);
	btn->StretchHorFillParent();
	return btn;
}