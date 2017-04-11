#pragma once
#include <BaseLibrary\Common_tmp.hpp>
#include "GuiText.hpp"

namespace inl::gui {

class GuiButton : public Widget
{
public:
	GuiButton(GuiEngine* guiEngine);
	GuiButton(const GuiButton& other) { *this = other; }

	// Important to implement in derived classes
	virtual GuiButton* Clone() const override { return new GuiButton(*this); }
	GuiButton& operator = (const GuiButton& other);

	//void SetBackgroundToColor(Color& idleColor, Color& hoverColor);
	void SetText(const std::wstring& text);
	void SetText(const std::string& text);
	void SetTextAlign(eTextAlign align);

public:
	GuiText* text;
};

inline GuiButton::GuiButton(GuiEngine* guiEngine)
:Widget(guiEngine)
{
	text = AddText();
}

inline GuiButton& GuiButton::operator = (const GuiButton& other)
{
	Widget::operator = (other);

	text = GetChildByIdx<GuiText>(other.text->GetIndexInParent());

	return *this;
}

inline void GuiButton::SetText(const std::wstring& str)
{
	text->SetText(str);
}

inline void GuiButton::SetText(const std::string& str)
{
	text->SetText(str);
}

inline void GuiButton::SetTextAlign(eTextAlign align)
{
	text->SetAlign(align);
}

} // namespace inl::gui