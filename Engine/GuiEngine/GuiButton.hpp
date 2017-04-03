#pragma once
#include <BaseLibrary\Common_tmp.hpp>
#include "GuiText.hpp"

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

	onTransformChange += [](Widget* selff, Rect<float>& rect)
	{
		selff->AsButton()->text->SetRect(rect);
	};
}

inline GuiButton& GuiButton::operator = (const GuiButton& other)
{
	Widget::operator = (other);

	text = GetChildByIdx<GuiText>(other.text->GetIdx());

	return *this;
}

inline void GuiButton::SetText(const std::wstring& str)
{
	text->Set(str);
}

inline void GuiButton::SetText(const std::string& str)
{
	text->Set(str);
}

inline void GuiButton::SetTextAlign(eTextAlign align)
{
	text->SetAlign(align);
}
