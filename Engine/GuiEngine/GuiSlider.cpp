#pragma once
#include "GuiSlider.hpp"
#include "GuiEngine.hpp"

GuiSlider::GuiSlider(GuiEngine* guiEngine)
:GuiControl(guiEngine), value(0), minValue(0), maxValue(1), sliderWidth(5), bSliding(false)
{
	background = AddPlane();
	slider = AddPlane();
	slider->SetIdleColor(Color(130, 130, 130));
	slider->SetHoverColor(slider->GetIdleColor());
	background->SetHoverColor(background->GetIdleColor());

	onTransformChange += [](GuiControl* selff, Rect<float>& rect)
	{
		GuiSlider* self = selff->AsSlider();

		self->background->SetRect(rect);
		self->SlideToValue();
	};

	onMouseEnter += [](GuiControl* selff, CursorEvent& evt)
	{
		GuiSlider* self = selff->AsSlider();
		self->slider->SetActiveColor(self->slider->GetIdleColor() + 65);
		self->background->SetActiveColor(self->background->GetIdleColor() + 35);
	};

	onMouseLeave += [](GuiControl* selff, CursorEvent& evt)
	{
		GuiSlider* self = selff->AsSlider();
		self->slider->SetActiveColor(self->slider->GetIdleColor());
		self->background->SetActiveColor(self->background->GetIdleColor());
	};

	// Start drag
	onMousePress += [](GuiControl* selff, CursorEvent& evt)
	{
		GuiSlider* self = selff->AsSlider();
		self->bSliding = true;
		self->SlideToCursor();
	};

	// Dragging
	guiEngine->onMouseMove += [&](CursorEvent& evt)
	{
		if (bSliding)
			SlideToCursor();
	};

	// Stop draw
	guiEngine->onMouseRelease += [&](CursorEvent& evt)
	{
		bSliding = false;
	};

	SlideToValue();
}