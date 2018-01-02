#pragma once
#include "BaseLibrary\Common.hpp"
#include "Gui.hpp"

namespace inl::ui {

class GuiSlider : public Gui
{
public:
	GuiSlider(GuiEngine& guiEngine);

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
	Delegate<void(Gui& self, float value)> OnValueChange;

protected:
	Gui* slider;

	float value;
	float minValue;
	float maxValue;
	float sliderWidth;

	bool bSliding;
};

} // namespace inl::ui