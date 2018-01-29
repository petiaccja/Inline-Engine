#pragma once
#include "List.hpp"
#include "Button.hpp"
#include <unordered_map>

namespace inl::gui {

class Menu : public ListView
{
public:
	Menu(GuiEngine& guiEngine);

	using ListView::AddItem;
	virtual void AddItem(Gui* gui) override;

	Menu* AddItemMenu(const std::wstring& text);
	Menu* AddItemMenu(const std::string& text) { return AddItemMenu(std::wstring(text.begin(), text.end())); }

	Button* GetGuiArrow()	{ return guiArrow; }
	Button* GetButton()		{ return guiButton; }
	Text* GetButtonText()	{ return guiButton->GetText(); }
	
protected:
	void SetGuiArrow(Button* arrow) { guiArrow = arrow; }
	void SetGuiButton(Button* btn) { guiButton = btn; }

protected:
	std::unordered_map<Gui*, Menu*> subMenus;
	Button* guiArrow;
	Button* guiButton;
};

} // namespace inl::gui
