#pragma once
#include "GuiSlider.hpp"
#include "GuiEngine.hpp"

//void GuiSlider::OnUpdated()
//{
//	// TODO unomptimal to do for every guiSlider's update
//	if (guiEngine->GetActiveSlider() == this)
//	{
//		float normalizedPercent = GetClientCursorPosX() / GetWidth();
//		normalizedPercent = saturate(normalizedPercent);
//		value = minValue + normalizedPercent * (maxValue - minValue);
//		UpdateSliderFromValue();
//	}
//}
//
void GuiSlider::OnPressed(CursorEvent& evt)
{
	guiEngine->SetActiveSlider(this);

	//
	onUpdate += [](GuiControl* selff, float deltaTime)
	{
		GuiSlider* self = selff->AsSlider();
		float& value = self->value;
		float& minValue = self->minValue;
		float& maxValue = self->maxValue;

		float normalizedPercent = self->GetClientCursorPosX() / self->GetWidth();
		normalizedPercent = saturate(normalizedPercent);
		value = minValue + normalizedPercent * (maxValue - minValue);
		self->UpdateSliderFromValue();
	};
}