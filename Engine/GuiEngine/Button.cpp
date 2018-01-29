#include "Button.hpp"

namespace inl::gui {

Button::Button(GuiEngine& guiEngine)
:Gui(guiEngine)
{
	text = AddGui<Text>();
	text->StretchFitToContent();
	text->AlignLeft();
	text->AlignVerCenter();
	text->DisableHover();
}

Button& Button::operator = (const Button& other)
{
	Gui::operator = (other);

	text = Copy(other.text);

	return *this;
}

void Button::SetText(const std::wstring& str)
{
	text->SetText(str);
}

void Button::SetText(const std::string& str)
{
	text->SetText(str);
}

} // namespace inl::gui