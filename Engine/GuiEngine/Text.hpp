#pragma once
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

class Text : public Gui
{
public:
	Text(GuiEngine* guiEngine);
	Text(const Text& other):Gui(other.guiEngine) { *this = other; }

	virtual Text* Clone() const { return new Text(*this); }
	Text& operator = (const Text& other);

	//virtual void OnPaint(Gdiplus::Graphics* graphics, GuiRectF& clipRect) override;

	void SetFontSize(int size);
	void SetFontFamily(const std::wstring& text);
	void SetFontFamily(const std::string& text) { SetFontFamily(std::wstring(text.begin(), text.end())); }
	void SetFontStyle(Gdiplus::FontStyle style);

	virtual Vec2 ArrangeChildren() override;

	void SetText(const std::wstring& text);
	void SetText(const std::string& text);

protected:
	std::wstring text;
	ColorI color;
	int fontSize;
	std::wstring fontFamilyName;
	Gdiplus::FontStyle fontStyle;

	std::unique_ptr<Gdiplus::Font> font;
	std::unique_ptr<Gdiplus::FontFamily> fontFamily;
};




inline Text::Text(GuiEngine* guiEngine)
:Gui(guiEngine), color(220, 220, 220, 255), fontSize(0)
{
	SetFontFamily("Helvetica");
	SetFontSize(12);
	SetFontStyle(Gdiplus::FontStyle::FontStyleRegular);

	HideBgImage();
	HideBgColor();

	OnPaint += [](PaintEvent& e)
	{
		Text& self = e.self->As<Text>();
	
		if (self.text.length() == 0)
			return;
	
		//auto rect = self.GetContentRect();
		auto visibleContentRect = self.GetVisibleContentRect();
	
		// TODO visibleRect
		//Gdiplus::RectF gdiClipRect = Gdiplus::RectF(rect.left, rect.top, rect.GetWidth(), rect.GetHeight());
		Gdiplus::RectF gdiClipRect = Gdiplus::RectF(visibleContentRect.left, visibleContentRect.top, visibleContentRect.GetWidth(), visibleContentRect.GetHeight());
	
		// Clipping (INTERSECT MODE)
		//graphics->SetClip(gdiClipRect, Gdiplus::CombineMode::CombineModeIntersect);
		//graphics->SetClip(gdiClipRect, Gdiplus::CombineMode::CombineModeReplace);
	
		ColorI color = self.color;
		Gdiplus::SolidBrush brush(Gdiplus::Color(color.a, color.r, color.g, color.b));
	
		Gdiplus::PointF pointF(self.GetPos().x, self.GetPos().y);
	
		e.graphics->SetTextRenderingHint(Gdiplus::TextRenderingHintSystemDefault);
		e.graphics->DrawString(self.text.c_str(), -1, self.font.get(), pointF, &brush);
	};
}

inline Text& Text::operator = (const Text& other)
{
	Gui::operator=(other);

	text = other.text;
	color = other.color;

	SetFontFamily(other.fontFamilyName);
	SetFontSize(other.fontSize);
	SetFontStyle(other.fontStyle);

	return *this;
}

inline void Text::SetFontSize(int size)
{
	if (fontSize != size)
	{
		fontSize = size;
		font.reset(new Gdiplus::Font(fontFamily.get(), fontSize, fontStyle, Gdiplus::UnitPixel));
	}
}

inline void Text::SetFontFamily(const std::wstring& text)
{
	if (fontFamilyName != text)
	{
		fontFamilyName = text;
		fontFamily.reset(new Gdiplus::FontFamily(text.c_str()));
	}
}

inline void Text::SetFontStyle(Gdiplus::FontStyle style)
{
	if (fontStyle != style)
	{
		fontStyle = style;
		font.reset(new Gdiplus::Font(fontFamily.get(), fontSize, fontStyle, Gdiplus::UnitPixel));
	}
}

inline void Text::SetText(const std::wstring& text)
{
	this->text = text;
}

inline void Text::SetText(const std::string& text)
{
	// Conversion to wchar_t, TODO replace with utf8 lib
	SetText(std::wstring(text.begin(), text.end()));
}

} // namespace inl::gui