#pragma once
#include "BaseLibrary\Common.hpp"
#include "GuiGrid.hpp"

namespace inl::gui {

class GuiScrollable : public GuiGrid
{
public:
	GuiScrollable(GuiEngine& guiEngine);
	GuiScrollable(const GuiScrollable& other):GuiGrid(other.guiEngine) { *this = other; }

	//virtual void AddItem(Gui* gui) {};
	//virtual bool RemoveItem(Gui* gui) { return false; };
	//virtual std::vector<Gui*> GetItems() { return std::vector<Gui*>(); }

	void SetContent(Gui* contentGui);


	// Important to implement in derived classes
	//virtual GuiScrollable* Clone() const override { return new GuiScrollable(*this); }
	//
	//virtual void AddItem(Gui* gui) override { Add(gui); }
	//virtual bool RemoveItem(Gui* gui) override { return Remove(gui); }
	//virtual std::vector<Gui*> GetItems() override { return GetChildren(); };
	//
	//void MakeVertical() { SetOrientation(eGuiOrientation::VERTICAL); }
	//void MakeHorizontal() { SetOrientation(eGuiOrientation::HORIZONTAL); }
	//
	//void SetOrientation(eGuiOrientation dir);
	//eGuiOrientation GetOrientation() { return orientation; }

//protected:
//	virtual Vec2 ArrangeChildren(const Vec2& finalSize) override;

protected:
	bool bVerScrollBarVisible;
	bool bHorScrollBarVisible;
};

} // namespace inl::gui