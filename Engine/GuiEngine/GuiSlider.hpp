#pragma once
#include <BaseLibrary\Common_tmp.hpp>
#include "GuiControl.hpp"
#include "GuiPlane.hpp"

class GuiSlider : public GuiControl
{
public:
	GuiSlider(GuiEngine* guiEngine);

	// Important to implement in derived classes
	virtual GuiSlider* Clone() const override { return new GuiSlider(*this); }

	void SetValue(float val) { value = val; }
	void SetMinValue(float val) { minValue = val; }
	void SetMaxValue(float val) { maxValue = val; }

protected:
	void UpdateSliderFromValue();
	void OnPressed(CursorEvent& evt);

protected:
	GuiPlane* background;
	GuiPlane* slider;

	float value;
	float minValue;
	float maxValue;
	float sliderWidth;
};

inline GuiSlider::GuiSlider(GuiEngine* guiEngine)
:GuiControl(guiEngine), value(0), minValue(0), maxValue(1), sliderWidth(5)
{
	background = AddPlane();
	slider = AddPlane();
	slider->SetBaseColor(Color(100, 100, 100));
	slider->SetHoverColor(Color(100, 100, 100));

	onTransformChanged += [](GuiControl* selff, Rect<float>& rect)
	{
		GuiSlider* self = selff->AsSlider();

		self->background->SetRect(rect);
		self->UpdateSliderFromValue();
	};

	//OnUpdate += [](GuiControl* selff, float deltaTime)
	//{
	//	GuiSlider* self = selff->AsSlider();
	//	self->onUpdated();
	//};
	//
	onPress += [](GuiControl* selff, CursorEvent& evt)
	{
		GuiSlider* self = selff->AsSlider();
		self->OnPressed(evt);
	};
	//
	

	UpdateSliderFromValue();
}

inline void GuiSlider::UpdateSliderFromValue()
{
	float normedPercent = value / (maxValue - minValue);
	slider->SetSize(sliderWidth, GetHeight());
	slider->SetPos(GetPosX() + normedPercent * (GetWidth() - slider->GetWidth()), GetPosY());
}