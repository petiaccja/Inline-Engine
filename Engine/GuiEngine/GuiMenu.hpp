#pragma once
#include "GuiList.hpp"
#include "GuiButton.hpp"
#include <unordered_map>

namespace inl::gui {

class GuiMenu : public GuiList
{
public:
	GuiMenu(GuiEngine* guiEngine):GuiList(guiEngine)
	{
		StretchFitToChildren();
	}

	virtual void AddItem(Gui* gui);

	GuiMenu* AddItemMenu(const std::wstring& text);
	GuiMenu* AddItemMenu(const std::string& text) { return AddItemMenu(std::wstring(text.begin(), text.end())); }

protected:
	std::unordered_map<Gui*, GuiMenu*> subMenus;
};

} // namespace inl::gui