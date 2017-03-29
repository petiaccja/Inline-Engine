#pragma once
#include <BaseLibrary\Common_tmp.hpp>
#include "GuiControl.hpp"

enum class eGuiListDirection
{
	VERTICAL,
	HORIZONTAL,
};

class GuiList : public GuiControl
{
public:
	GuiList(GuiEngine* guiEngine);

	// Important to implement in derived classes
	virtual GuiList* Clone() const override { return new GuiList(*this); }

	void SetStride(float val) { stride = val; }

protected:
	float stride;
	eGuiListDirection eDirection;
};

inline GuiList::GuiList(GuiEngine* guiEngine)
:GuiControl(guiEngine), eDirection(eGuiListDirection::VERTICAL)
{
	onTransformChange += [](GuiControl* selff, Rect<float>& rect)
	{
		GuiList* self = selff->AsList();

		int i = 0;
		for (GuiControl* list : self->GetChildren())
		{
			if (self->eDirection == eGuiListDirection::VERTICAL)
			{
				list->SetRect(self->GetPosX(), self->GetPosY() + i * self->stride, self->GetWidth(), self->stride);
			}
			else
			{
				list->SetRect(self->GetPosX() + i * self->stride, self->GetPosY(), self->stride, self->GetHeight());
			}
			++i;
		}
	};
}