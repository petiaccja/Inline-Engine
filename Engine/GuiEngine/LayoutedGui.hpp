#pragma once
#include "Gui.hpp"

namespace inl::gui
{

class LayoutedGui : public Gui
{
public:
	LayoutedGui() {}
	LayoutedGui(GuiEngine* guiEngine) :Gui(guiEngine) {}

	virtual void AddItem(Gui* gui) = 0;
	virtual bool RemoveItem(Gui* gui) = 0;
	virtual std::vector<Gui*> GetItems() = 0;

	template<class T>
	T* AddItem();

	Gui*			AddItemGui();
	GuiText*		AddItemText();
	GuiButton*		AddItemButton(const std::string& text = "");
	GuiList*		AddItemList();
	GuiSlider*		AddItemSlider();
	GuiCollapsable* AddItemCollapsable();
	GuiSplitter*	AddItemSplitter();
	Gui*			AddItemSeparatorHor();
};

} // namespace inl::gui
