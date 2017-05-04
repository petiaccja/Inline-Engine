#pragma once
#include "LayoutedGui.hpp"
#include "GuiText.hpp"
#include "GuiButton.hpp"
#include "GuiList.hpp"
#include "GuiSlider.hpp"
#include "GuiCollapsable.hpp"
#include "GuiSplitter.hpp"

using namespace inl::gui;

Gui* LayoutedGui::AddItemGui()
{
	return AddItem<Gui>();
}

GuiText* LayoutedGui::AddItemText()
{
	return AddItem<GuiText>();
}

GuiButton* LayoutedGui::AddItemButton(const std::string& text /*= ""*/)
{
	GuiButton* btn = AddItem<GuiButton>();
	btn->SetText(text);
	return btn;
}

GuiList* LayoutedGui::AddItemList()
{
	return AddItem<GuiList>();
}

GuiSlider* LayoutedGui::AddItemSlider()
{
	return AddItem<GuiSlider>();
}

GuiCollapsable* LayoutedGui::AddItemCollapsable()
{
	return AddItem<GuiCollapsable>();
}

GuiSplitter* LayoutedGui::AddItemSplitter()
{
	return AddItem<GuiSplitter>();
}

Gui* LayoutedGui::AddItemSeparatorHor()
{
	Gui* btn = AddItemGui();
	btn->SetSize(1, 1);
	btn->StretchHorFillParent();
	return btn;
}