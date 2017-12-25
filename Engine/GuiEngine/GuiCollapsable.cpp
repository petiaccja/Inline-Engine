#pragma once
#include "GuiCollapsable.hpp"

using namespace inl::gui;

GuiCollapsable::GuiCollapsable(GuiEngine& guiEngine)
:GuiList(guiEngine), bOpened(false)
{
	//StretchFitToChildren();
	//SetAutoSize(true);

	list = new GuiList(guiEngine);
	//list->SetBgToColor(Color(0, 0, 0, 0));

	caption = AddGui<GuiButton>();
	//caption->SetAlign(eGuiAlign::STRETCH_H);

	caption->OnCursorPressed += [](Gui& self, CursorEvent& e)
	{
		GuiCollapsable& c = self.GetParent()->As<GuiCollapsable>();

		if (c.bOpened)
			c.RemoveGui(c.list);
		else
			c.AddGui(c.list);

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

	caption = Copy(other.caption);
	list = Copy(other.list);

	bOpened = other.bOpened;

	return *this;
}