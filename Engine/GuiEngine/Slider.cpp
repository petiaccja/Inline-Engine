#pragma once
#include "Slider.hpp"
#include "GuiEngine.hpp"

namespace inl::gui {

Slider::Slider(GuiEngine* guiEngine)
:Gui(guiEngine), value(0), minValue(0), maxValue(1), sliderWidth(5), bSliding(false)
{
	slider = AddGui();
	slider->SetBgIdleColor(ColorI(130, 130, 130, 255));
	slider->SetBgHoverColor(slider->GetBgIdleColor());

	OnTransformChange += [](TransformEvent& e)
	{
		Slider& self = e.self->As<Slider>();
		self.SlideToValue();
	};

	OnCursorEnter += [](CursorEvent& e)
	{
		Slider& self = e.self->As<Slider>();
		self.slider->SetBgActiveColor(self.slider->GetBgIdleColor() + ColorI(65, 65, 65, 0));
	};

	OnCursorLeave += [](CursorEvent& e)
	{
		Slider& self = e.self->As<Slider>();
		self.slider->SetBgActiveColorToIdle();
	};

	// Start drag
	OnCursorPress += [](CursorEvent& e)
	{
		Slider& self = e.self->As<Slider>();
		self.bSliding = true;
		self.SlideToCursor();
	};

	// Dragging
	guiEngine->OnCursorMove += [this](CursorEvent& e)
	{
		if (bSliding)
			SlideToCursor();
	};

	// Stop draw
	guiEngine->OnCursorRelease += [this](CursorEvent& e)
	{
		bSliding = false;
	};

	SlideToValue();
}

void Slider::SlideToValue(float value)
{
	float normedPercent = value / (maxValue - minValue);
	SlideToNormedPercent(normedPercent);
}

void Slider::SlideToNormedPercent(float normedPercent)
{
	slider->SetSize(sliderWidth, GetSize().y);
	slider->SetPos(GetPosX() + normedPercent * (GetWidth() - slider->GetWidth()), GetPosY());
}

void Slider::SlideToCursor()
{
	// Ha cursor sliderHalfWidth() - nél van akkor 0, ha GetWidth() - sliderHalfWidth() - nél akkor meg egy
	float normalizedPercent = (GetCursorPosContentSpaceX() - slider->GetHalfWidth()) / (GetWidth() - slider->GetWidth());
	normalizedPercent = Saturate(normalizedPercent);
	SlideToNormedPercent(normalizedPercent);
}

void Slider::SetValue(float val)
{
	value = Clamp(val, minValue, maxValue);
	SlideToValue(value);
	OnValueChange(*this, val);
}

void Slider::SetMinValue(float val)
{
	minValue = val;
	val = Clamp(val, minValue, maxValue);
	SetValue(val);
}

void Slider::SetMaxValue(float val)
{
	maxValue = val;
	val = Clamp(val, minValue, maxValue);
	SetValue(val);
}

} //namespace inl::gui