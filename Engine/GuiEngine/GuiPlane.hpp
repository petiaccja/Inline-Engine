#pragma once
#include <BaseLibrary\Common_tmp.hpp>
#include "GuiControl.hpp"

class GuiPlane : public GuiControl
{
public:
	GuiPlane(){}

	void SetColor(Color& color);

	virtual void OnPaint(HDC dc, Gdiplus::Graphics* graphics) override;

protected:
	Color color;
};

inline void GuiPlane::SetColor(Color& color)
{
	this->color = color;
}

inline void GuiPlane::OnPaint(HDC dc, Gdiplus::Graphics* graphics)
{
	Gdiplus::SolidBrush  brush(Gdiplus::Color(color.a, color.r, color.g, color.b));
	graphics->FillRectangle(&brush, Gdiplus::Rect(rect.x, rect.y, rect.width, rect.height));
}