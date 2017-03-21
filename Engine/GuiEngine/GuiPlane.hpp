#pragma once
#include <BaseLibrary\Common_tmp.hpp>
#include "GuiControl.hpp"

class GuiPlane : public GuiControl
{
public:
	GuiPlane();

	virtual void OnPaint(HDC dc, Gdiplus::Graphics* graphics) override;

	void SetActiveColor(Color& color);
	void SetBaseColor(Color& color);
	void SetHoverColor(Color& color);

protected:
	Color activeColor;

	Color baseColor;
	Color hoverColor;
};

inline GuiPlane::GuiPlane()
:baseColor(55, 55, 55), hoverColor(80, 80, 80)
{
	SetActiveColor(baseColor);

	OnCursorEnter += [&](CursorEvent& event){
		SetActiveColor(hoverColor);
	};

	OnCursorLeave += [&](CursorEvent& event) {
		SetActiveColor(baseColor);
	};

	OnParentChanged += [&](GuiControl* parent)
	{
		parent->OnTransformChanged += [&](Rect<float>& rect)
		{
			SetRect(rect);
		};
	};
}

inline void GuiPlane::OnPaint(HDC dc, Gdiplus::Graphics* graphics)
{
	Gdiplus::SolidBrush  brush(Gdiplus::Color(activeColor.a, activeColor.r, activeColor.g, activeColor.b));
	graphics->FillRectangle(&brush, Gdiplus::Rect(rect.x, rect.y, rect.width, rect.height));
}

inline void GuiPlane::SetActiveColor(Color& color)
{
	activeColor = color;
}

inline void GuiPlane::SetBaseColor(Color& color)
{
	baseColor = color;
	SetActiveColor(baseColor);
}

inline void GuiPlane::SetHoverColor(Color& color)
{
	hoverColor = color;
}