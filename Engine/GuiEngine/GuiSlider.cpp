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

	onTransformChanged += [this](RectF& rect)
	{
		SlideToValue();
	};

	onMouseEntered += [this](CursorEvent& evt)
	{
		slider->SetBgActiveColor(slider->GetBgIdleColor() + 65);
	};

	onMouseLeaved += [this](CursorEvent& evt)
	{
		slider->SetBgActiveColorToIdle();
	};

	// Start drag
	onMousePressed += [this](CursorEvent& evt)
	{
		bSliding = true;
		SlideToCursor();
	};

	// Dragging
	guiEngine->onMouseMove += [this](CursorEvent& evt)
	{
		if (bSliding)
			SlideToCursor();
	};

	// Stop draw
	guiEngine->onMouseRelease += [this](CursorEvent& evt)
	{
		bSliding = false;
	};

	SlideToValue();
}