#pragma once
#include "GuiSlider.hpp"
#include "GuiEngine.hpp"

GuiSlider::GuiSlider(GuiEngine* guiEngine)
:Widget(guiEngine), value(0), minValue(0), maxValue(1), sliderWidth(5), bSliding(false)
{
	slider = AddWidget();
	slider->SetBgIdleColor(Color(130));
	slider->SetBgHoverColor(slider->GetBgIdleColor());

	onTransformChange += [](Widget* selff, Rect<float>& rect)
	{
		GuiSlider* self = selff->AsSlider();

		self->SlideToValue();
	};

	onMouseEnter += [](Widget* selff, CursorEvent& evt)
	{
		GuiSlider* self = selff->AsSlider();
		self->slider->SetBgActiveColor(self->slider->GetBgIdleColor() + 65);
	};

	onMouseLeave += [](Widget* selff, CursorEvent& evt)
	{
		GuiSlider* self = selff->AsSlider();
		self->slider->SetBgActiveColorToIdle();
	};

	// Start drag
	onMousePress += [](Widget* selff, CursorEvent& evt)
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