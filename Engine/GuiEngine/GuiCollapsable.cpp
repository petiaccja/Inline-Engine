#pragma once
#include "GuiCollapsable.hpp"

using namespace inl::ui;

GuiCollapsable::GuiCollapsable(GuiEngine& guiEngine)
:GuiLayout(guiEngine), bOpened(false)
{
	DisableHover();

	StretchHorFillParent();
	StretchVerFitToChildren();

	// List layout for caption and item list
	layout = AddGui<GuiList>();
	layout->StretchFillParent();

	// Create item list
	itemList = new GuiList(guiEngine);
	itemList->StretchHorFillParent();
	itemList->OnChildAdd += [](Gui& self, ChildEvent& e)
	{
		e.child->StretchHorFillParent();
		e.child->StretchVerFitToChildren();
		e.child->SetPadding(4);
	};

	// Add caption to layout
	caption = layout->AddItem<GuiButton>();
	caption->StretchHorFillParent();
	caption->StretchVerFitToChildren();
	caption->SetBgIdleColor(ColorI(25, 25, 25, 255));
	caption->SetBgHoverColor(ColorI(60, 60, 60, 255));
	caption->GetGuiText()->SetFontSize(16);
	caption->SetPadding(4);

	caption->OnCursorPress += [](Gui& self, CursorEvent& e)
	{
		GuiCollapsable& c = self.GetParent()->GetParent()->As<GuiCollapsable>();

		// Add item list to layout based on if "collapsable" opened or not
		if (c.bOpened)
			c.layout->RemoveItem(c.itemList);
		else
			c.layout->AddItem(c.itemList);

		c.bOpened = !c.bOpened;
	};
}

void GuiCollapsable::SetCaptionText(const std::wstring& str)
{
	caption->SetText(str);
}

GuiCollapsable& GuiCollapsable::operator = (const GuiCollapsable& other)
{
	Gui::operator = (other);

	layout = Copy(other.layout);
	caption = Copy(other.caption);
	itemList = Copy(other.itemList);

	bOpened = other.bOpened;

	return *this;
}