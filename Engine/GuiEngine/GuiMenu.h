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

	GuiMenu* AddItemMenu(const std::wstring& text);
	GuiMenu* AddItemMenu(const std::string& text) { return AddItemMenu(std::wstring(text.begin(), text.end())); }
};

} // namespace inl::gui