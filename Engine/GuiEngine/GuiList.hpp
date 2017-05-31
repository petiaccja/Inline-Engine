#pragma once
#include <BaseLibrary\Common_tmp.hpp>
#include "GuiLayout.hpp"

namespace inl::gui {

class GuiList : public GuiLayout
{
public:
	GuiList(GuiEngine* guiEngine);
	GuiList(const GuiList& other) { *this = other; }

	// Important to implement in derived classes
	virtual GuiList* Clone() const override { return new GuiList(*this); }

	virtual void AddItem(Gui* gui) override { Add(gui); }
	virtual bool RemoveItem(Gui* gui) override { return Remove(gui); }
	virtual std::vector<Gui*> GetItems() override { return GetChildren(); };

	void MakeVertical() { SetOrientation(eGuiOrientation::VERTICAL); }
	void MakeHorizontal() { SetOrientation(eGuiOrientation::HORIZONTAL); }

	void SetOrientation(eGuiOrientation dir);
	eGuiOrientation GetOrientation() { return orientation; }

protected:
	virtual Vector2f ArrangeChildren(const Vector2f& finalSize) override;

protected:
	eGuiOrientation orientation;
};

} // namespace inl::gui