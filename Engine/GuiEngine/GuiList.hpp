#pragma once
#include <BaseLibrary\Common_tmp.hpp>
#include "GuiControl.hpp"

class GuiList : public GuiControl
{
public:
	GuiList(GuiEngine* guiEngine);

	// Important to implement in derived classes
	virtual GuiList* Clone() const override { return new GuiList(*this); }

	void SetStride(float val) { stride = val; }

protected:
	float stride;
};

inline GuiList::GuiList(GuiEngine* guiEngine)
:GuiControl(guiEngine)
{
	
}