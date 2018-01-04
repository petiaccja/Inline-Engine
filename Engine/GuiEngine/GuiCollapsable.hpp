#pragma once
#include "GuiList.hpp"
#include "GuiButton.hpp"

namespace inl::ui {

class GuiCollapsable : public GuiLayout
{
public:
	GuiCollapsable(GuiEngine& guiEngine);
	GuiCollapsable(const GuiCollapsable& other):GuiLayout(other.guiEngine) { *this = other; }

	// Important to implement in derived classes
	virtual GuiCollapsable* Clone() const override { return new GuiCollapsable(*this); }
	GuiCollapsable& operator = (const GuiCollapsable& other);

	using GuiLayout::AddItem;
	virtual void AddItem(Gui* gui) override { itemList->AddItem(gui); }
	virtual bool RemoveItem(Gui* gui) override { return itemList->RemoveItem(gui); }
	virtual std::vector<Gui*> GetItems() override  { return itemList->GetItems(); };

	void SetCaptionText(const std::wstring& str);

	GuiButton* GetCaption() { return caption; }

protected:
	// Layout
	GuiList* layout;

	// Layout items
	GuiButton* caption;
	GuiList* itemList;

	bool bOpened;
};

} //namespace inl::ui