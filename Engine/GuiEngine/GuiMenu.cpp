#include "GuiMenu.h"

using namespace inl::gui;

GuiMenu* GuiMenu::AddItemMenu(const std::wstring& text)
{
	GuiButton* btn = new GuiButton(guiEngine);
	btn->SetText(text);

	GuiMenu* subMenu = new GuiMenu(guiEngine);
	btn->SetPrivateData(subMenu);

	AddItem(btn);

	return subMenu;
}

void GuiMenu::AddItem(Gui* gui)
{
	GuiList::AddItem(gui);

	if (GetOrientation() == eGuiOrientation::VERTICAL)
		gui->StretchHorFillParent();
	else
		gui->StretchVerFillParent();

	GuiMenu* menu = gui->GetPrivateData<GuiMenu>();

	static std::vector<Gui*> activeMenuTree;
	static Gui* activeMenuBtn = nullptr;
	gui->onMouseEnteredClonable += [menu](Gui* self, CursorEvent& evt)
	{
		// Case 1. It's menu, close menus behind this and open that one
		// case 2. It's not a menu, close menus behind this

		// Close menus behind this
		auto it = activeMenuTree.begin();
		while (it != activeMenuTree.end())
		{
			Gui* menuBtn = *it;

			bool bSibling = menuBtn->IsSibling(self);

			if (bSibling)
			{
				while (it != activeMenuTree.end())
				{
					Gui* menuBtn = *it;
					GuiMenu* menuToClose = menuBtn->GetPrivateData<GuiMenu>();

					if (menuToClose)
						menuToClose->RemoveFromParent();

					it = activeMenuTree.erase(it);
				}
				break;
			}
			++it;
		}

		// If that button have menu assigned, OPEN it
		if (menu)
		{
			menu->BringToFront();

			GuiMenu* containingMenu = self->GetParent()->AsMenu();

			if (containingMenu->GetOrientation() == eGuiOrientation::HORIZONTAL)
				menu->SetPos(self->GetPosBottomLeft()); // TODO new menu should open at different position, for example menuBar open menus down ! and average menus opens to the right
			else
				menu->SetPos(self->GetPosTopRight()); // TODO new menu should open at different position, for example menuBar open menus down ! and average menus opens to the right

			activeMenuTree.push_back(self);
		}
	};
}