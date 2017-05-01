#pragma once
#include <BaseLibrary\Common_tmp.hpp>
#include "LayoutedGui.hpp"
#include <unordered_set>

namespace inl::gui {

class GuiSplitter : public LayoutedGui
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

protected:
	virtual Vector2f ArrangeChildren(const Vector2f& finalSize) override;

protected:
	std::unordered_set<Gui*> items;
	eGuiOrientation orientation;
	int separatorLength;
};

} // namespace inl::gui