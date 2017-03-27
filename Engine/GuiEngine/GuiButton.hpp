#pragma once
#include <BaseLibrary\Common_tmp.hpp>
#include "GuiControl.hpp"
#include "GuiPlane.hpp"
#include "GuiText.hpp"

class GuiButton : public GuiControl
{
public:
	GuiButton(GuiEngine* guiEngine);

	~GuiButton();
	void Clear();

	// Important to implement in derived classes
	virtual GuiButton& operator = (const GuiButton& other);
	virtual GuiButton* Clone() const override { return new GuiButton(*this); }

	void SetBackgroundToColor(Color& baseColor, Color& hoverColor);
	void SetText(const std::wstring& text);
	void SetText(const std::string& text);
	void SetTextAlign(eTextAlign align);

public:
	GuiPlane* background;
	GuiText*  text;
};

inline GuiButton::GuiButton(GuiEngine* guiEngine)
:GuiControl(guiEngine), background(AddPlane()), text(AddText())
{
	//background = AddPlane();
	//text = AddText();

	OnTransformChanged += [](GuiControl* selff, Rect<float>& rect)
	{
		selff->AsButton()->background->SetRect(rect);//TODO REMOVE
	};
}

inline GuiButton::~GuiButton()
{
	Clear();
}

inline void GuiButton::Clear()
{
	delete background;
	delete text;
}

inline GuiButton& GuiButton::operator = (const GuiButton& other)
{
	Clear();

	background = other.background->Clone();
	text = other.text->Clone();

	GuiControl::operator = (other);

	return *this;
}

inline void GuiButton::SetBackgroundToColor(Color& baseColor, Color& hoverColor)
{
	background->SetBaseColor(baseColor);
	background->SetHoverColor(hoverColor);
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
