#pragma once
#include <BaseLibrary\Common_tmp.hpp>
#include "GuiLayouted.hpp"

namespace inl::gui {

class GuiScrollable : public GuiLayouted
{
public:
	GuiScrollable(GuiEngine* guiEngine);
	GuiScrollable(const GuiScrollable& other) { *this = other; }

	virtual void AddItem(Gui* gui) {};
	virtual bool RemoveItem(Gui* gui) { return false; };
	virtual std::vector<Gui*> GetItems() { return std::vector<Gui*>(); }

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
//	virtual Vector2f ArrangeChildren(const Vector2f& finalSize) override;

protected:
	eGuiOrientation orientation;

	Gui* contentGui;
	GuiList* listHor;
	GuiList* listVer;
};

} // namespace inl::gui