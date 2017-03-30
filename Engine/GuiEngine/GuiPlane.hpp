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
	void SetIdleColor(Color& color);
	void SetHoverColor(Color& color);

	Color& GetActiveColor() { return activeColor; }
	Color& GetIdleColor() { return idleColor; }
	Color& GetHoverColor() { return hoverColor; }

protected:
	Color activeColor;
	Color idleColor;
	Color hoverColor;
};

inline GuiPlane::GuiPlane(GuiEngine* guiEngine)
:GuiControl(guiEngine), idleColor(45, 45, 45), hoverColor(75, 75, 75)
{
	SetActiveColor(idleColor);

	onMouseEnter += [](GuiControl* selff, CursorEvent& event)
	{
		GuiPlane* self = selff->AsPlane();
		self->SetActiveColor(self->hoverColor);
	};

	onMouseLeave += [](GuiControl* selff, CursorEvent& event)
	{
		GuiPlane* self = selff->AsPlane();
		self->SetActiveColor(self->idleColor);
	};

	onParentTransformChange += [&](GuiControl* self, Rect<float>& rect)
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

inline void GuiPlane::SetIdleColor(Color& color)
{
	idleColor = color;
	SetActiveColor(idleColor);
}

inline void GuiPlane::SetHoverColor(Color& color)
{
	hoverColor = color;
}