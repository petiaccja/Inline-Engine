#pragma once
#include <BaseLibrary\Common_tmp.hpp>
#include "Gui.hpp"


namespace inl::gui {

enum class eTextAlign
{
	LEFT,
	RIGHT,
	TOP,
	BOTTOM,
	CENTER,
	TOP_LEFT,
	TOP_RIGHT,
	BOTTOM_LEFT,
	BOTTOM_RIGHT,
};

class GuiText : public Gui
{
public:
	GuiText(GuiEngine* guiEngine);
	GuiText(const GuiText& other) { *this = other; }

	virtual GuiText* Clone() const { return new GuiText(*this); }
	GuiText& operator = (const GuiText& other);

	virtual void OnPaint(Gdiplus::Graphics* graphics, RectF& clipRect) override;

	void SetFontSize(int size);
	void SetFontFamily(const std::wstring& text);
	void SetFontFamily(const std::string& text) { SetFontFamily(std::wstring(text.begin(), text.end())); }
	void SetFontStyle(Gdiplus::FontStyle style);

	virtual Vector2f GuiText::ArrangeChildren(const Vector2f& finalSize) override;

	void SetText(const std::wstring& text);
	void SetText(const std::string& text);

protected:
	std::wstring text;
	Color color;
	int fontSize;
	std::wstring fontFamilyName;
	Gdiplus::FontStyle fontStyle;

	std::unique_ptr<Gdiplus::Font> font;
	std::unique_ptr<Gdiplus::FontFamily> fontFamily;
};




inline GuiText::GuiText(GuiEngine* guiEngine)
:Gui(guiEngine), color(Color::WHITE)
{
	SetFontFamily("Helvetica");
	SetFontSize(12);
	SetFontStyle(Gdiplus::FontStyle::FontStyleRegular);

	HideBgImage();
	HideBgColor();
}

inline GuiText& GuiText::operator = (const GuiText& other)
{
	Gui::operator=(other);

	text = other.text;
	color = other.color;

	SetFontFamily(other.fontFamilyName);
	SetFontSize(other.fontSize);
	SetFontStyle(other.fontStyle);

	return *this;
}

inline void GuiText::SetFontSize(int size)
{
	if (fontSize != size)
	{
		fontSize = size;
		font.reset(new Gdiplus::Font(fontFamily.get(), fontSize, fontStyle, Gdiplus::UnitPixel));
	}
}

inline void GuiText::SetFontFamily(const std::wstring& text)
{
	if (fontFamilyName != text)
	{
		fontFamilyName = text;
		fontFamily.reset(new Gdiplus::FontFamily(text.c_str()));
	}
}

inline void GuiText::SetFontStyle(Gdiplus::FontStyle style)
{
	if (fontStyle != style)
	{
		fontStyle = style;
		font.reset(new Gdiplus::Font(fontFamily.get(), fontSize, fontStyle, Gdiplus::UnitPixel));
	}
}

inline void GuiText::SetText(const std::wstring& text)
{
	this->text = text;
}

inline void GuiText::SetText(const std::string& text)
{
	// Conversion to wchar_t, TODO replace with utf8 lib
	SetText(std::wstring(text.begin(), text.end()));
}

inline void GuiText::OnPaint(Gdiplus::Graphics* graphics, RectF& clipRect)
{
	Gui::OnPaint(graphics, clipRect);

	if (text.length() == 0)
		return;

	auto rect = GetContentRect();

	Gdiplus::RectF gdiClipRect = Gdiplus::RectF(rect.left, rect.top, rect.GetWidth(), rect.GetHeight());

	// Clipping (INTERSECT MODE)
	//graphics->SetClip(gdiClipRect, Gdiplus::CombineMode::CombineModeIntersect);

	Gdiplus::SolidBrush brush(Gdiplus::Color(color.r, color.g, color.b, color.a));

	Gdiplus::PointF pointF(GetPosX(), GetPosY());

	graphics->SetTextRenderingHint(Gdiplus::TextRenderingHintSystemDefault);
	graphics->DrawString(text.c_str(), -1, font.get(), pointF, &brush);
};

} // namespace inl::gui