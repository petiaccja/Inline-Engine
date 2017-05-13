#pragma once
#include "Gui.hpp"

namespace inl::gui
{

class GuiLayouted : public Gui
{
public:
	GuiLayouted() {}
	GuiLayouted(GuiEngine* guiEngine) :Gui(guiEngine) {}

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
	GuiImage*		AddItemImage();
};

template<class T>
T* GuiLayouted::AddItem()
{
	T* child = new T(guiEngine);
	AddItem(child);
	return child;

}
} // namespace inl::gui
