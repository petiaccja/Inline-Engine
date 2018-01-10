#pragma once
#include "GuiList.hpp"

namespace inl::gui {

class GuiGrid;
class GuiButton;
class GuiImage;

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
	GuiGrid* layout;

	// Layout items
	GuiButton* caption;
	GuiImage* arrow;
	GuiList* itemList;

	bool bOpened;
};

} //namespace inl::gui