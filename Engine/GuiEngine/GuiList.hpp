#pragma once
#include <BaseLibrary\Common_tmp.hpp>
#include "Widget.hpp"

namespace inl::gui {

enum class eGuiListDirection
{
	VERTICAL,
	HORIZONTAL,
};

class GuiList : public Widget
{
public:
	GuiList(GuiEngine* guiEngine);
	GuiList(const GuiList& other) { *this = other; }

	// Important to implement in derived classes
	virtual GuiList* Clone() const override { return new GuiList(*this); }

	void SetDirection(eGuiListDirection dir);

	eGuiListDirection GetDirection() { return direction; }

protected:
	void ArrangeChilds();

protected:
	eGuiListDirection direction;
};

inline GuiList::GuiList(GuiEngine* guiEngine)
	:Widget(guiEngine), direction(eGuiListDirection::VERTICAL)
{
	SetFitToChildren(true);

	SetBgColorForAllStates(Color(0, 0, 0, 255));

	onChildAdded += [](Widget* selff, Widget* child)
	{
		GuiList* self = selff->AsList();
		child->SetFitToChildren(true);
		self->ArrangeChilds();
	};

	onChildRemoved += [](Widget* selff, Widget* child)
	{
		GuiList* self = selff->AsList();
		self->ArrangeChilds();
	};

	onTransformChanged += [](Widget* selff, RectF& rect)
	{
		GuiList* self = selff->AsList();
		self->ArrangeChilds();
	};

	onChildTransformChanged += [](Widget* selff, RectF& rect)
	{
		GuiList* self = selff->AsList();
		self->ArrangeChilds();
	};
}

inline void GuiList::SetDirection(eGuiListDirection dir)
{
	direction = dir;
	ArrangeChilds();
}

inline void GuiList::ArrangeChilds()
{
	int i = 0;
	vec2 finalSize(0, 0);
	float distance = 0;
	float maxDiameter = 0;
	for (Widget* child : GetChildren())
	{
		if (direction == eGuiListDirection::VERTICAL)
		{
			//child->SetRect(GetClientPosX(), GetClientPosY() + finalSize.y, child->GetWidth(), child->GetHeight());
			child->SetPos(GetClientPosX(), GetClientPosY() + finalSize.y);
			finalSize.y += child->GetHeight();
			finalSize.x = std::max(finalSize.x, child->GetWidth());
		}
		else if (direction == eGuiListDirection::HORIZONTAL)
		{
			//child->SetRect(GetClientPosX() + finalSize.x, GetClientPosY(), child->GetWidth(), child->GetHeight());
			child->SetPos(GetClientPosX() + finalSize.x, GetClientPosY());
			finalSize.x += child->GetWidth();
			finalSize.y = std::max(finalSize.y, child->GetHeight());
		}
		++i;
	}
	//SetClientSize(finalSize);
}

} // namespace inl::gui