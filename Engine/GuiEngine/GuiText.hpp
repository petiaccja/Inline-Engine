#pragma once
#include <BaseLibrary\Common_tmp.hpp>
#include "GuiPlane.hpp"

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

class GuiText : public GuiControl
{
public:
	GuiText();

	void Set(const std::wstring& text);
	void Set(const std::string& text);
	
	virtual void OnPaint(HDC dc, Gdiplus::Graphics* graphics) override;

protected:
	std::wstring text;
	Color color;
	int fontSize;
	eTextAlign textAlign;
};

inline GuiText::GuiText()
:color(Color::WHITE), fontSize(12), textAlign(eTextAlign::CENTER)
{
	
}

inline void GuiText::Set(const std::wstring& text)
{
	this->text = text;
}

inline void GuiText::Set(const std::string& text)
{
	// Conversion to wchar_t
	this->text = std::wstring(text.begin(), text.end());
}

inline void GuiText::OnPaint(HDC hdc, Gdiplus::Graphics* graphics)
{
	Gdiplus::SolidBrush  brush(Gdiplus::Color(color.r, color.g, color.b, color.a));
	Gdiplus::FontFamily  fontFamily(L"Helvetica");
	Gdiplus::Font        font(&fontFamily, fontSize, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);

	vec2 textPos;
	switch (textAlign)
	{
		case eTextAlign::CENTER:
		{
			Gdiplus::RectF layoutRect(rect.x, rect.y, rect.width, rect.height);
			Gdiplus::RectF boundRect;
			graphics->MeasureString(text.c_str(), text.size(), &font, layoutRect, &boundRect);

			textPos = rect.GetCenter();
			textPos.x -= boundRect.Width * 0.5f;
			textPos.y -= boundRect.Height * 0.5f;

			break;
		}
		default:__debugbreak();
	}

	Gdiplus::PointF pointF(textPos.x, textPos.y);

	graphics->SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);
	graphics->DrawString(text.c_str(), -1, &font, pointF, &brush);
}