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
	GuiText(GuiEngine* guiEngine);

	virtual GuiText* Clone() const { return new GuiText(*this); }

	void Set(const std::wstring& text);
	void Set(const std::string& text);
	void SetAlign(eTextAlign align);

protected:
	std::wstring text;
	Color color;
	int fontSize;
	eTextAlign align;
};

inline GuiText::GuiText(GuiEngine* guiEngine)
:GuiControl(guiEngine), color(Color::WHITE), fontSize(12), align(eTextAlign::CENTER)
{
	onParentTransformChanged += [&](GuiControl* self, Rect<float>& rect)
	{
		self->AsText()->SetRect(rect);
	};

	onPaint += [](GuiControl* selff, HDC hdc, Gdiplus::Graphics* graphics)
	{
		GuiText* self = selff->AsText();
		auto text = self->text;
		auto color = self->color;
		auto fontSize = self->fontSize;
		auto align = self->align;
		auto rect = self->GetRect();
	
		Gdiplus::SolidBrush  brush(Gdiplus::Color(color.r, color.g, color.b, color.a));
		Gdiplus::FontFamily  fontFamily(L"Helvetica");
		Gdiplus::Font        font(&fontFamily, fontSize, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
	
		// Text alignment
		Gdiplus::RectF gdiRect(rect.x, rect.y, rect.width, rect.height);
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
			textPos.x = rect.x;
			textPos.y = rect.GetCenter().y - textRect.Height * 0.5f;
		} break;
		case eTextAlign::RIGHT:
		{
			textPos.x = rect.GetRight() - textRect.Width;
			textPos.y = rect.GetCenter().y - textRect.Height * 0.5f;
		} break;
		case eTextAlign::TOP:
		{
			textPos.x = rect.GetCenter().x - textRect.Width * 0.5f;
			textPos.y = rect.GetTop();
		} break;
		case eTextAlign::BOTTOM:
		{
			textPos.x = rect.GetCenter().x - textRect.Width * 0.5f;
			textPos.y = rect.GetBottom() - textRect.Height;
		} break;
		case eTextAlign::TOP_LEFT:
		{
			textPos.x = rect.GetLeft();
			textPos.y = rect.GetTop();
		} break;
		case eTextAlign::TOP_RIGHT:
		{
			textPos.x = rect.GetRight() - textRect.Width;
			textPos.y = rect.GetTop();
		} break;
		case eTextAlign::BOTTOM_LEFT:
		{
			textPos.x = rect.GetLeft();
			textPos.y = rect.GetBottom() - textRect.Height;
		} break;
		case eTextAlign::BOTTOM_RIGHT:
		{
			textPos.x = rect.GetRight() - textRect.Width;
			textPos.y = rect.GetBottom() - textRect.Height;
		} break;
		default:__debugbreak();
		}
	
		Gdiplus::PointF pointF(textPos.x, textPos.y);
	
		graphics->SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);
		graphics->DrawString(text.c_str(), -1, &font, pointF, &brush);
	};
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

inline void GuiText::SetAlign(eTextAlign align)
{
	this->align = align;
}