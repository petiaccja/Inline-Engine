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

	virtual GuiText* Clone() const { return new GuiText(*this); }
	virtual void OnPaint(Gdiplus::Graphics* graphics, RectF& clipRect) override;

	virtual Vector2f MeasureChildren(const Vector2f& availableSize) override;

	void SetText(const std::wstring& text);
	void SetText(const std::string& text);
	//void SetTextAlign(eTextAlign align);

protected:
	std::wstring text;
	Color color;
	int fontSize;
	//eTextAlign textAlign;
};

inline GuiText::GuiText(GuiEngine* guiEngine)
:Gui(guiEngine), color(Color::WHITE), fontSize(14)
{
	HideBgImage();
	HideBgColor();
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

//inline void GuiText::SetTextAlign(eTextAlign align)
//{
//	this->textAlign = align;
//}

inline void GuiText::OnPaint(Gdiplus::Graphics* graphics, RectF& clipRect)
{
	Gui::OnPaint(graphics, clipRect);

	if (text.length() == 0)
		return;

	auto rect = GetClientRect();

	Gdiplus::RectF gdiClipRect = Gdiplus::RectF(rect.left, rect.top, rect.GetWidth(), rect.GetHeight());

	// Clipping (INTERSECT MODE)
	graphics->SetClip(gdiClipRect, Gdiplus::CombineMode::CombineModeIntersect);

	Gdiplus::SolidBrush  brush(Gdiplus::Color(color.r, color.g, color.b, color.a));
	Gdiplus::FontFamily  fontFamily(L"Helvetica");
	Gdiplus::Font        font(&fontFamily, fontSize, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);

	// Text alignment
	Gdiplus::RectF gdiRect(rect.left, rect.top, rect.GetWidth(), rect.GetHeight());
	Gdiplus::RectF textRect;
	graphics->MeasureString(text.c_str(), text.size(), &font, gdiRect, &textRect);

	// Position text to center
	Vector2f textPos;
	textPos = Vector2f(textRect.GetLeft() + textRect.GetRight(), textRect.GetTop() + textRect.GetBottom()) * 0.5f;
	textPos.x() -= textRect.Width * 0.5f;
	textPos.y() -= textRect.Height * 0.5f;

	//switch (textAlign)
	//{
	//case eTextAlign::CENTER:
	//{
	//	textPos = rect.GetCenter();
	//	textPos.x() -= textRect.Width * 0.5f;
	//	textPos.y() -= textRect.Height * 0.5f;
	//
	//} break;
	//case eTextAlign::LEFT:
	//{
	//	textPos.x() = rect.left;
	//	textPos.y() = rect.GetCenter().y() - textRect.Height * 0.5f;
	//} break;
	//case eTextAlign::RIGHT:
	//{
	//	textPos.x() = rect.right - textRect.Width;
	//	textPos.y() = rect.GetCenter().y() - textRect.Height * 0.5f;
	//} break;
	//case eTextAlign::TOP:
	//{
	//	textPos.x() = rect.GetCenter().x() - textRect.Width * 0.5f;
	//	textPos.y() = rect.top;
	//} break;
	//case eTextAlign::BOTTOM:
	//{
	//	textPos.x() = rect.GetCenter().x() - textRect.Width * 0.5f;
	//	textPos.y() = rect.bottom - textRect.Height;
	//} break;
	//case eTextAlign::TOP_LEFT:
	//{
	//	textPos.x() = rect.left;
	//	textPos.y() = rect.top;
	//} break;
	//case eTextAlign::TOP_RIGHT:
	//{
	//	textPos.x() = rect.right - textRect.Width;
	//	textPos.y() = rect.top;
	//} break;
	//case eTextAlign::BOTTOM_LEFT:
	//{
	//	textPos.x() = rect.left;
	//	textPos.y() = rect.bottom - textRect.Height;
	//} break;
	//case eTextAlign::BOTTOM_RIGHT:
	//{
	//	textPos.x() = rect.right - textRect.Width;
	//	textPos.y() = rect.bottom - textRect.Height;
	//} break;
	//default:__debugbreak();
	//}

	Gdiplus::PointF pointF(textPos.x(), textPos.y());

	graphics->SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);
	graphics->DrawString(text.c_str(), -1, &font, pointF, &brush);
};

inline Vector2f GuiText::MeasureChildren(const Vector2f& availableSize)
{
	Gdiplus::Graphics* graphics = Gdiplus::Graphics::FromHDC(GetDC(NULL));

	Gdiplus::FontFamily  fontFamily(L"Helvetica");
	Gdiplus::Font        font(&fontFamily, fontSize, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);

	Gdiplus::RectF gdiRect(0,0, availableSize.x(), availableSize .y());
	Gdiplus::RectF textRect;
	graphics->MeasureString(text.c_str(), text.size(), &font, gdiRect, &textRect);

	return Vector2f(round(textRect.Width), round(textRect.Height));
}

} // namespace inl::gui