#pragma once
#include "GuiList.hpp"
#include "GuiButton.hpp"
#include <unordered_map>

namespace inl::gui {

class GuiMenu : public GuiList
{
public:
	GuiMenu(GuiEngine& guiEngine);

	using GuiList::AddItem;
	virtual void AddItem(Gui* gui) override;

	GuiMenu* AddItemMenu(const std::wstring& text);
	GuiMenu* AddItemMenu(const std::string& text) { return AddItemMenu(std::wstring(text.begin(), text.end())); }

	GuiButton* GetGuiArrow() { return guiArrow; }
	GuiText* GetGuiButtonText() { return guiButton->GetGuiText(); }
	GuiButton* GetGuiButton() { return guiButton; }
	
protected:
	void SetGuiArrow(GuiButton* arrow) { guiArrow = arrow; }
	void SetGuiButton(GuiButton* btn) { guiButton = btn; }

protected:
	std::unordered_map<Gui*, GuiMenu*> subMenus;
	GuiButton* guiArrow;
	GuiButton* guiButton;
};

} // namespace inl::gui
