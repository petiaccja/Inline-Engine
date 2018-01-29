#pragma once
#include "Gui.hpp"

namespace inl::gui {

class Slider : public Gui
{
public:
	Slider(GuiEngine& guiEngine);

	// Important to implement in derived classes
	virtual Slider* Clone() const override { return new Slider(*this); }

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

} // namespace inl::gui
