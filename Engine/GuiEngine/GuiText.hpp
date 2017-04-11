#pragma once
#include <BaseLibrary\Common_tmp.hpp>
#include "Widget.hpp"

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

class GuiText : public Widget
{
public:
	GuiText(GuiEngine* guiEngine);

	virtual GuiText* Clone() const { return new GuiText(*this); }

	void SetText(const std::wstring& text);
	void SetText(const std::string& text);
	void SetAlign(eTextAlign align);

protected:
	void ArrangeText(Gdiplus::Graphics* graphics);

protected:
	std::wstring text;
	Color color;
	int fontSize;
	eTextAlign align;
};

inline GuiText::GuiText(GuiEngine* guiEngine)
:Widget(guiEngine), color(Color::WHITE), fontSize(14), align(eTextAlign::CENTER)
{
	HideBgImage();
	HideBgColor();

	onPaint += [](Widget* selff, Gdiplus::Graphics* graphics, RectF& clipRect)
	{
		GuiText* self = selff->AsText();
		auto text = self->text;

		if (text.length() == 0)
			return;

		self->ArrangeText(graphics);

		auto color = self->color;
		auto fontSize = self->fontSize;
		auto align = self->align;
		auto rect = self->GetClientRect();

		Gdiplus::SolidBrush  brush(Gdiplus::Color(color.r, color.g, color.b, color.a));
		Gdiplus::FontFamily  fontFamily(L"Helvetica");
		Gdiplus::Font        font(&fontFamily, fontSize, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
	
		// Text alignment
		Gdiplus::RectF gdiRect(rect.left, rect.top, rect.GetWidth(), rect.GetHeight());
		Gdiplus::RectF textRect;
		graphics->MeasureString(text.c_str(), text.size(), &font, gdiRect, &textRect);

		vec2 textPos;
		switch (align)
		{
		case eTextAlign::CENTER:
		{
			textPos = rect.GetCenter();
			textPos.x -= textRect.Width * 0.5f;
			textPos.y -= textRect.Height * 0.5f;
	
		} break;
		case eTextAlign::LEFT:
		{
			textPos.x = rect.left;
			textPos.y = rect.GetCenter().y - textRect.Height * 0.5f;
		} break;
		case eTextAlign::RIGHT:
		{
			textPos.x = rect.right - textRect.Width;
			textPos.y = rect.GetCenter().y - textRect.Height * 0.5f;
		} break;
		case eTextAlign::TOP:
		{
			textPos.x = rect.GetCenter().x - textRect.Width * 0.5f;
			textPos.y = rect.top;
		} break;
		case eTextAlign::BOTTOM:
		{
			textPos.x = rect.GetCenter().x - textRect.Width * 0.5f;
			textPos.y = rect.bottom - textRect.Height;
		} break;
		case eTextAlign::TOP_LEFT:
		{
			textPos.x = rect.left;
			textPos.y = rect.top;
		} break;
		case eTextAlign::TOP_RIGHT:
		{
			textPos.x = rect.right - textRect.Width;
			textPos.y = rect.top;
		} break;
		case eTextAlign::BOTTOM_LEFT:
		{
			textPos.x = rect.left;
			textPos.y = rect.bottom - textRect.Height;
		} break;
		case eTextAlign::BOTTOM_RIGHT:
		{
			textPos.x = rect.right - textRect.Width;
			textPos.y = rect.bottom - textRect.Height;
		} break;
		default:__debugbreak();
		}

		Gdiplus::PointF pointF(textPos.x, textPos.y);
	
		graphics->SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);
		graphics->DrawString(text.c_str(), -1, &font, pointF, &brush);
	};
}

inline void GuiText::SetText(const std::wstring& text)
{
	this->text = text;
}

inline void GuiText::SetText(const std::string& text)
{
	// Conversion to wchar_t, TODO replace with utf8 lib
	this->text = std::wstring(text.begin(), text.end());
}

inline void GuiText::SetAlign(eTextAlign align)
{
	this->align = align;
	//SetClientSize(100, 20);
}

inline void GuiText::ArrangeText(Gdiplus::Graphics* graphics)
{
	Gdiplus::FontFamily  fontFamily(L"Helvetica");
	Gdiplus::Font        font(&fontFamily, fontSize, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
	
	Gdiplus::RectF gdiRect(-999999, -999999, 9999999, 9999999);
	Gdiplus::RectF textRect;
	graphics->MeasureString(text.c_str(), text.size(), &font, gdiRect, &textRect);
	
	SetClientSize(ceil(textRect.Width), ceil(textRect.Height));
}