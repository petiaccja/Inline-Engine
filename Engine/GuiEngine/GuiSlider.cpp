#pragma once
#include "GuiSlider.hpp"
#include "GuiEngine.hpp"

GuiSlider::GuiSlider(GuiEngine* guiEngine)
:GuiControl(guiEngine), value(0), minValue(0), maxValue(1), sliderWidth(5), bSliding(false)
{
	background = AddPlane();
	slider = AddPlane();
	slider->SetBaseColor(Color(100, 100, 100));
	slider->SetHoverColor(Color(100, 100, 100));

	onTransformChange += [](GuiControl* selff, Rect<float>& rect)
	{
		GuiSlider* self = selff->AsSlider();

		self->background->SetRect(rect);
		self->SlideToValue();
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