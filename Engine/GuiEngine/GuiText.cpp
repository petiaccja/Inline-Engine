#include "GuiText.hpp"
#include "GuiEngine.hpp"

using namespace inl::gui;

Vector2f GuiText::ArrangeChildren(const Vector2f& finalSize)
{
	Gdiplus::RectF gdiRect(-FLT_MAX * 0.5, -FLT_MAX * 0.5, FLT_MAX, FLT_MAX);

	Gdiplus::RectF textRect;
	guiEngine->GetGdiGraphics()->MeasureString(text.c_str(), text.size(), font.get(), gdiRect, &textRect);

	return Vector2f(round(textRect.Width), round(textRect.Height));
}