#pragma once
#include "GuiCollapsable.hpp"

GuiCollapsable::GuiCollapsable(GuiEngine* guiEngine)
:GuiList(guiEngine), bOpened(false)
{
	SetFitToChildren(true);

	list = new GuiList(guiEngine);
	list->SetBgColorForAllStates(Color(0, 0, 0, 0));

	caption = AddButton();

	caption->onMousePressed += [](Widget* selff, CursorEvent& evt)
	{
		GuiCollapsable* self = selff->GetParent()->AsCollapsable();

		if (self->bOpened)
			self->Remove(self->list);
		else
			self->Add(self->list);

		self->bOpened = !self->bOpened;
	};
}