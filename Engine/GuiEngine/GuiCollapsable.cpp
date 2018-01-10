#pragma once
#include "GuiCollapsable.hpp"
#include "GuiGrid.hpp"
#include "GuiButton.hpp"
#include "GuiImage.hpp"

using namespace inl::gui;

GuiCollapsable::GuiCollapsable(GuiEngine& guiEngine)
:GuiLayout(guiEngine), bOpened(false)
{
	DisableHover();

	StretchHorFillParent();
	StretchVerFitToContent();


	// Grid layout for arrow, caption, and the item list		arrow | caption
	//															empty |	item 0
	//															empty | item 1
	//															empty | item ...

	layout = AddGui<GuiGrid>();
	layout->StretchFillParent();
	layout->SetDimension(2, 1);

	// Arrow
	arrow = layout->GetCell(0, 0)->AddGui<GuiImage>();
	arrow->SetImages(L"Resources/empty_right_arrow.png", L"Resources/empty_right_arrow.png", 12, 12);
	arrow->DisableHover();
	arrow->AlignCenter();

	layout->GetRow(0)->StretchFitToContent();
	layout->GetColumn(0)->SetWidth(30);
	layout->GetColumn(1)->StretchFillSpace(1.0);

	// item list
	itemList = new GuiList(guiEngine);
	itemList->OnChildAdd += [](Gui& self, ChildEvent& e)
	{
		e.child->StretchHorFillParent();
		e.child->StretchVerFitToContent();
		e.child->SetPadding(2);
	};

	// Caption
	caption = layout->GetCell(1, 0)->AddGui<GuiButton>();
	caption->StretchHorFillParent();
	caption->StretchVerFitToContent();
	caption->SetBgIdleColor(ColorI(25, 25, 25, 255));
	caption->SetBgHoverColor(ColorI(60, 60, 60, 255));
	caption->GetGuiText()->SetFontSize(16);
	caption->SetPadding(4);

	caption->OnCursorPress += [](Gui& self, CursorEvent& e)
	{
		GuiCollapsable& c = self.GetParent()->GetParent()->GetParent()->As<GuiCollapsable>();
	
		// Add item list to layout based on if "collapsable" opened or not
		if (c.bOpened)
		{
			c.layout->SetDimension(2, 1);
		}
		else
		{
			c.layout->SetDimension(2, 2);
			c.layout->GetRow(1)->StretchFitToContent();

			c.layout->GetCell(1, 1)->AddGui(c.itemList);
			c.itemList->SetPos(c.layout->GetCell(1, 1)->GetPos());
		}
	
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
	arrow = Copy(other.arrow);

	bOpened = other.bOpened;

	return *this;
}