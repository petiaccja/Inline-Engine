#include "GuiText.hpp"
#include "GuiEngine.hpp"

using namespace inl::ui;

Vec2 GuiText::ArrangeChildren(const Vec2& finalSize)
{
	Gdiplus::RectF gdiRect(-FLT_MAX * 0.5, -FLT_MAX * 0.5, FLT_MAX, FLT_MAX);

	Gdiplus::RectF textRect;
	guiEngine.GetGdiGraphics()->MeasureString(text.c_str(), text.size(), font.get(), gdiRect, &textRect);

	return Vec2(round(textRect.Width), round(textRect.Height));
}