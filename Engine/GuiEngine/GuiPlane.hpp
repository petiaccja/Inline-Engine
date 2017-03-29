#pragma once
#include <BaseLibrary\Common_tmp.hpp>
#include "GuiControl.hpp"

class GuiPlane : public GuiControl
{
public:
	GuiPlane(GuiEngine* guiEngine);

	// Important to implement in derived classes
	virtual GuiPlane* Clone() const override { return new GuiPlane(*this); }

	void SetActiveColor(Color& color);
	void SetBaseColor(Color& color);
	void SetHoverColor(Color& color);

protected:
	Color activeColor;
	Color baseColor;
	Color hoverColor;
};

inline GuiPlane::GuiPlane(GuiEngine* guiEngine)
:GuiControl(guiEngine), baseColor(55, 55, 55), hoverColor(55, 55, 55)
{
	SetActiveColor(baseColor);

	onCursorEnter += [](GuiControl* selff, CursorEvent& event)
	{
		GuiPlane* self = selff->AsPlane();
		self->SetActiveColor(self->hoverColor);
	};

	onCursorLeave += [](GuiControl* selff, CursorEvent& event)
	{
		GuiPlane* self = selff->AsPlane();
		self->SetActiveColor(self->baseColor);
	};

	onParentTransformChanged += [&](GuiControl* self, Rect<float>& rect)
	{
		//self->AsPlane()->SetRect(rect);
	};

	onPaint += [](GuiControl* selff, HDC dc, Gdiplus::Graphics* graphics)
	{
		GuiPlane* self = selff->AsPlane();

		Gdiplus::SolidBrush  brush(Gdiplus::Color(self->activeColor.a, self->activeColor.r, self->activeColor.g, self->activeColor.b));
		graphics->FillRectangle(&brush, Gdiplus::Rect(self->GetPosX(), self->GetPosY(), self->GetWidth(), self->GetHeight()));
	};
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