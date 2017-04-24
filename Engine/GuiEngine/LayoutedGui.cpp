#pragma once
#include "LayoutedGui.hpp"
#include "GuiText.hpp"
#include "GuiButton.hpp"
#include "GuiList.hpp"
#include "GuiSlider.hpp"
#include "GuiCollapsable.hpp"
#include "GuiSplitter.hpp"

using namespace inl::gui;

template<class T>
T* LayoutedGui::AddItem()
{
	T* child = new T(guiEngine);
	AddItem(child);
	return child;
}

Gui* LayoutedGui::AddItemGui()
{
	return AddItem<Gui>();
}

GuiText* LayoutedGui::AddItemText()
{
	return AddItem<GuiText>();
}

GuiButton* LayoutedGui::AddItemButton()
{
	return AddItem<GuiButton>();
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