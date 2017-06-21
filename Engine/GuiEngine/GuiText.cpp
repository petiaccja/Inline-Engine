#include "GuiText.hpp"
#include "GuiEngine.hpp"

using namespace inl::gui;

GuiText::GuiText(GuiEngine* guiEngine)
:Gui(guiEngine), color(235, 235, 235, 255)
{
	SetFontFamily("Helvetica");
	SetFontSize(12);
	SetFontStyle(Gdiplus::FontStyle::FontStyleRegular);

	SetBgToColor(Color(0, 0, 0, 0));

	onPaintClonable += [](Gui* self_, Gdiplus::Graphics* graphics)
	{
		GuiText* self = self_->AsText();

		if (self->text.length() == 0)
			return;

		auto visibleContentRect = self->GetVisibleContentRect();

		Gdiplus::RectF gdiClipRect = Gdiplus::RectF(visibleContentRect.left, visibleContentRect.top, visibleContentRect.GetWidth(), visibleContentRect.GetHeight());

		Color color = self->color;
		Gdiplus::SolidBrush brush(Gdiplus::Color(color.a, color.r, color.g, color.b));

		Gdiplus::PointF pointF(self->GetPosX(), self->GetPosY());

		graphics->SetTextRenderingHint(Gdiplus::TextRenderingHintSystemDefault);
		graphics->DrawString(self->text.c_str(), -1, self->font.get(), pointF, &brush);
	};
}

GuiText& GuiText::operator = (const GuiText& other)
{
	Gui::operator=(other);

	text = other.text;
	color = other.color;

	SetFontFamily(other.fontFamilyName);
	SetFontSize(other.fontSize);
	SetFontStyle(other.fontStyle);

	return *this;
}

Vector2f GuiText::ArrangeChildren(const Vector2f& finalSize)
{
	Gdiplus::RectF gdiRect(-FLT_MAX * 0.5, -FLT_MAX * 0.5, FLT_MAX, FLT_MAX);

	Gdiplus::RectF textRect;
	guiEngine->GetGdiGraphics()->MeasureString(text.c_str(), text.size(), font.get(), gdiRect, &textRect);

	return Vector2f(round(textRect.Width), round(textRect.Height));
}

void GuiText::SetFontSize(int size)
{
	if (fontSize != size)
	{
		fontSize = size;
		font.reset(new Gdiplus::Font(fontFamily.get(), fontSize, fontStyle, Gdiplus::UnitPixel));
	}
}

void GuiText::SetFontFamily(const std::wstring& text)
{
	if (fontFamilyName != text)
	{
		fontFamilyName = text;
		fontFamily.reset(new Gdiplus::FontFamily(text.c_str()));
	}
}

void GuiText::SetFontStyle(Gdiplus::FontStyle style)
{
	if (fontStyle != style)
	{
		fontStyle = style;
		font.reset(new Gdiplus::Font(fontFamily.get(), fontSize, fontStyle, Gdiplus::UnitPixel));
	}
}

void GuiText::SetText(const std::wstring& text)
{
	this->text = text;
}

void GuiText::SetText(const std::string& text)
{
	// Conversion to wchar_t, TODO replace with utf8 lib
	SetText(std::wstring(text.begin(), text.end()));
}