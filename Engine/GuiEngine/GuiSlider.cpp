#pragma once
#include "GuiSlider.hpp"
#include "GuiEngine.hpp"

GuiSlider::GuiSlider(GuiEngine* guiEngine)
:Widget(guiEngine), value(0), minValue(0), maxValue(1), sliderWidth(5), bSliding(false)
{
	slider = AddWidget();
	slider->SetBgIdleColor(Color(130));
	slider->SetBgHoverColor(slider->GetBgIdleColor());

	onTransformChanged += [](Widget* selff, RectF& rect)
	{
		GuiSlider* self = selff->AsSlider();
		self->SlideToValue();
	};

	onMouseEntered += [](Widget* selff, CursorEvent& evt)
	{
		GuiSlider* self = selff->AsSlider();
		self->slider->SetBgActiveColor(self->slider->GetBgIdleColor() + 65);
	};

	onMouseLeaved += [](Widget* selff, CursorEvent& evt)
	{
		GuiSlider* self = selff->AsSlider();
		self->slider->SetBgActiveColorToIdle();
	};

	// Start drag
	onMousePressed += [](Widget* selff, CursorEvent& evt)
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