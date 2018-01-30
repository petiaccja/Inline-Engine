#pragma once
#include "Layout.hpp"

#include <unordered_set>

namespace inl::gui {

class Splitter : public Layout
{
public:
	Splitter(GuiEngine* guiEngine);
	Splitter(const Splitter& other):Layout(other.guiEngine) { *this = other; }

	// Important to implement in derived classes
	virtual Splitter* Clone() const override { return new Splitter(*this); }

	virtual void AddItem(Gui* gui) override;
	virtual bool RemoveItem(Gui* gui) override;
	virtual std::vector<Gui*> GetItems() override;

	void SetOrientation(eGuiOrientation dir);
	eGuiOrientation GetOrientation() { return orientation; }

	std::vector<Gui*>& GetSeparators() { return separators; }

protected:
	virtual Vec2 ArrangeChildren() override;

protected:
	std::unordered_set<Gui*> items;
	std::vector<Gui*> separators;

	eGuiOrientation orientation;
	int separatorLength;

	// For lambda states
	bool bDragging = false;
	Vec2 mousePosWhenPressed;
	Vec2 prevItemOrigSize;
	Vec2 nextItemOrigSize;
	Gui* separatorSaved;
};

} // namespace inl::gui