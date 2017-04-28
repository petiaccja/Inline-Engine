#pragma once
#include "GuiList.hpp"
#include "GuiButton.hpp"

namespace inl::gui {

class GuiMenu : public GuiList
{
public:
	GuiMenu(GuiEngine* guiEngine):GuiList(guiEngine)
	{
		StretchFitToChildren();
	}

	virtual void AddItem(Gui* gui)
	{
		GuiList::AddItem(gui);
		gui->StretchHorFillParent();
	}

	//GuiList(const GuiMenu& other) { *this = other; }
	//
	//// Important to implement in derived classes
	//virtual GuiList* Clone() const override { return new GuiList(*this); }

	GuiMenu* AddItemMenu(const std::wstring& text);
	GuiMenu* AddItemMenu(const std::string& text) { return AddItemMenu(std::wstring(text.begin(), text.end())); }
};


inline GuiMenu* GuiMenu::AddItemMenu(const std::wstring& text)
{
	GuiButton* btn = AddButton();
	btn->SetText(text);
	btn->StretchHorFillParent();

	GuiMenu* subMenu = CreateMenu();

	btn->onMouseEnteredClonable += [subMenu](Gui* self, CursorEvent& evt)
	{
		// btn hovered -> show subMenu at button's right side
		subMenu->BringToFront();
		subMenu->SetPos(self->GetPosBottomLeft());
	};

	//btn->onMouseLeavedClonable += [subMenu](Gui* self, CursorEvent& evt)
	//{
	//	// btn hovered -> show subMenu at button's right side
	//	subMenu->Remove();
	//};
	//
	//subMenu->onMouseLeavedClonable += [](Gui* self, CursorEvent& evt)
	//{
	//	// btn hovered -> show subMenu at button's right side
	//	self->Remove();
	//};

	return subMenu;
}

} // namespace inl::gui