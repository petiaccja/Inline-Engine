#pragma once
#include "GuiSlider.hpp"
#include "GuiEngine.hpp"

using namespace inl::gui;

GuiSlider::GuiSlider(GuiEngine* guiEngine)
:Gui(guiEngine), value(0), minValue(0), maxValue(1), sliderWidth(5), bSliding(false)
{
	slider = AddGui();
	slider->SetBgIdleColor(Color(130));
	slider->SetBgHoverColor(slider->GetBgIdleColor());

	onTransformChangedClonable += [](Gui* _self, RectF& rect)
	{
		GuiSlider* self = _self->AsSlider();
		self->SlideToValue();
	};

	onMouseEnteredClonable += [](Gui* _self, CursorEvent& evt)
	{
		GuiSlider* self = _self->AsSlider();
		self->slider->SetBgActiveColor(self->slider->GetBgIdleColor() + 65);
	};

	onMouseLeavedClonable += [](Gui* _self, CursorEvent& evt)
	{
		GuiSlider* self = _self->AsSlider();
		self->slider->SetBgActiveColorToIdle();
	};

	// Start drag
	onMousePressedClonable += [](Gui* _self, CursorEvent& evt)
	{
		GuiSlider* self = _self->AsSlider();
		self->bSliding = true;
		self->SlideToCursor();
	};

	// Dragging
	guiEngine->onMouseMoved += [this](CursorEvent& evt)
	{
		if (bSliding)
			SlideToCursor();
	};

	// Stop draw
	guiEngine->onMouseReleased += [this](CursorEvent& evt)
	{
		bSliding = false;
	};

	SlideToValue();
}