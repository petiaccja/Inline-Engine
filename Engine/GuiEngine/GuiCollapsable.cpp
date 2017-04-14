#pragma once
#include "GuiCollapsable.hpp"

using namespace inl::gui;

GuiCollapsable::GuiCollapsable(GuiEngine* guiEngine)
:GuiList(guiEngine), bOpened(false)
{
	//SetAutoSize(true);

	list = new GuiList(guiEngine);
	list->SetBgColorForAllStates(Color(0, 0, 0, 0));

	caption = AddButton();
	//caption->SetAlign(eGuiAlign::STRETCH_H);

	caption->onMousePressed += [this](CursorEvent& evt)
	{
		if (bOpened)
			Remove(list);
		else
			Add(list);

		bOpened = !bOpened;
	};
}