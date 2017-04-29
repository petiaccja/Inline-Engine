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

	virtual void AddItem(Gui* gui);

	//GuiList(const GuiMenu& other) { *this = other; }
	//
	//// Important to implement in derived classes
	//virtual GuiList* Clone() const override { return new GuiList(*this); }

	GuiMenu* AddItemMenu(const std::wstring& text);
	GuiMenu* AddItemMenu(const std::string& text) { return AddItemMenu(std::wstring(text.begin(), text.end())); }
};


inline GuiMenu* GuiMenu::AddItemMenu(const std::wstring& text)
{
	GuiButton* btn = CreateButton();
	btn->SetText(text);
	btn->StretchHorFillParent();

	GuiMenu* subMenu = CreateMenu();
	btn->SetPrivateData(subMenu);

	AddItem(btn);

	return subMenu;
}

inline void GuiMenu::AddItem(Gui* gui)
{
	GuiList::AddItem(gui);
	gui->StretchHorFillParent();

	GuiMenu* menu = gui->GetPrivateData<GuiMenu>();

	static std::vector<Gui*> activeMenuTree;
	static Gui* activeMenuBtn = nullptr;
	gui->onMouseEnteredClonable += [menu](Gui* self, CursorEvent& evt)
	{
		bool bRemove = false;

		// Search button in the active and opened MenuTree
		auto it = std::find(activeMenuTree.begin(), activeMenuTree.end(), self);
		if (it != activeMenuTree.end())
		{
			++it; // Step right, to the first "behind"
			bRemove = true;
		}
		else // Hm we didn't found the btn so we might open a new menu, OR this is a button without menu, so we need to close behind menu
		{
			// So we have a menu, open it
			if (menu)
			{
				menu->BringToFront();
				menu->SetPos(self->GetPosTopRight()); // TODO new menu should open at different position, for example menuBar open menus down ! and average menus opens to the right

				activeMenuTree.push_back(self);
				activeMenuBtn = self;
			}
			else
			{
				it = activeMenuTree.begin();
				while (it != activeMenuTree.end())
				{
					Gui* menuBtn = *it;

					bool isSelfNeighbour = menuBtn->IsChildNeighbour(self);

					if (isSelfNeighbour)
					{
						bRemove = true;
						break;
					}
					++it;
				}
			}
		}

		if (bRemove)
		{
			while (it != activeMenuTree.end())
			{
				Gui* menuBtn = *it;
				GuiMenu* menuToClose = menuBtn->GetPrivateData<GuiMenu>();

				if (menuToClose)
					menuToClose->Remove();

				it = activeMenuTree.erase(it);
			}
		}
	};
}

} // namespace inl::gui