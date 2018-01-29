#pragma once
#include "Layout.hpp"

namespace inl::gui {

class ListView : public Layout
{
public:
	ListView(GuiEngine& guiEngine);
	ListView(const ListView& other):Layout(other.guiEngine) { *this = other; }

	// Important to implement in derived classes
	virtual ListView* Clone() const override { return new ListView(*this); }

	using Layout::AddItem;
	virtual void AddItem(Gui* gui) override { AddGui(gui); }
	virtual bool RemoveItem(Gui* gui) override { return RemoveGui(gui); }
	virtual std::vector<Gui*> GetItems() override { return GetChildren(); };

	void MakeVertical() { SetOrientation(eGuiOrientation::VERTICAL); }
	void MakeHorizontal() { SetOrientation(eGuiOrientation::HORIZONTAL); }

	void SetOrientation(eGuiOrientation dir);
	eGuiOrientation GetOrientation() { return orientation; }

protected:
	virtual Vec2 ArrangeChildren() override;

protected:
	eGuiOrientation orientation;
};

} // namespace inl::gui