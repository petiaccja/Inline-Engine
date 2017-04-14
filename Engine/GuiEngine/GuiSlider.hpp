#pragma once
#include <BaseLibrary\Common_tmp.hpp>
#include "Gui.hpp"

namespace inl::gui {

class GuiSlider : public Gui
{
public:
	GuiSlider(GuiEngine* guiEngine);

	// Important to implement in derived classes
	virtual GuiSlider* Clone() const override { return new GuiSlider(*this); }

	void SetValue(float val);
	void SetMinValue(float val);
	void SetMaxValue(float val);

	void SlideToValue(float value);
	void SlideToNormedPercent(float normedPercent);

protected:
	void SlideToCursor();
	void SlideToValue() { SlideToValue(value); }

public:
	Delegate<void(Gui* self, float value)> OnValueChanged;

protected:
	Gui* background;
	Gui* slider;

	float value;
	float minValue;
	float maxValue;
	float sliderWidth;

	bool bSliding;
};

inline void GuiSlider::SlideToValue(float value)
{
	float normedPercent = value / (maxValue - minValue);
	SlideToNormedPercent(normedPercent);
}

inline void GuiSlider::SlideToNormedPercent(float normedPercent)
{
	slider->SetSize(sliderWidth, GetHeight());
	slider->SetPos(GetPosX() + normedPercent * (GetWidth() - slider->GetWidth()), GetPosY());
}

inline void GuiSlider::SlideToCursor()
{
	// Ha cursor sliderHalfWidth() - nél van akkor 0, ha GetWidth() - sliderHalfWidth() - nél akkor meg egy
	float normalizedPercent = (GetClientSpaceCursorPosX() - slider->GetHalfWidth()) / (GetWidth() - slider->GetWidth());
	normalizedPercent = saturate(normalizedPercent);
	SlideToNormedPercent(normalizedPercent);
}

inline void GuiSlider::SetValue(float val)
{
	value = clamp(val, minValue, maxValue);
	SlideToValue(value);
	OnValueChanged(this, val);
}

inline void GuiSlider::SetMinValue(float val)
{
	minValue = val;
	val = clamp(val, minValue, maxValue);
	SetValue(val);
}

inline void GuiSlider::SetMaxValue(float val)
{
	maxValue = val;
	val = clamp(val, minValue, maxValue);
	SetValue(val);
}

} // namespace inl::gui