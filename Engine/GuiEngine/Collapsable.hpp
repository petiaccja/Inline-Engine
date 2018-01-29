#pragma once
#include "List.hpp"
#include "Button.hpp"
namespace inl::gui {

class Grid;
//class Button;
class Image;

class Collapsable : public Layout
{
public:
	Collapsable(GuiEngine& guiEngine);
	Collapsable(const Collapsable& other):Layout(other.guiEngine) { *this = other; }

	// Important to implement in derived classes
	virtual Collapsable* Clone() const override { return new Collapsable(*this); }
	Collapsable& operator = (const Collapsable& other);

	using Layout::AddItem;
	virtual void AddItem(Gui* gui) override { itemList->AddItem(gui); }
	virtual bool RemoveItem(Gui* gui) override { return itemList->RemoveItem(gui); }
	virtual std::vector<Gui*> GetItems() override  { return itemList->GetItems(); };

	void SetCaptionText(const std::wstring& str);

	Button* GetCaption() { return caption; }

protected:
	// Layout
	Grid* layout;

	// Layout items
	Button* caption;
	Image* arrow;
	ListView* itemList;

	bool bOpened;
};

} //namespace inl::gui