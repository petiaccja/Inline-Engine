#pragma once
#include "BaseLibrary\Common.hpp"
#include "GuiLayout.hpp"

#include <unordered_set>

namespace inl::gui {

class GuiSplitter : public GuiLayout
{
public:
	GuiSplitter(GuiEngine* guiEngine);
	GuiSplitter(const GuiSplitter& other) { *this = other; }

	// Important to implement in derived classes
	virtual GuiSplitter* Clone() const override { return new GuiSplitter(*this); }

	virtual void AddItem(Gui* gui) override;
	virtual bool RemoveItem(Gui* gui) override;
	virtual std::vector<Gui*> GetItems() override;

	void SetOrientation(eGuiOrientation dir);
	eGuiOrientation GetOrientation() { return orientation; }

	std::vector<Gui*>& GetSeparators() { return separators; }

protected:
	virtual Vec2 ArrangeChildren(const Vec2& finalSize) override;

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