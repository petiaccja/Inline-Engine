#pragma once
#include <BaseLibrary\Common_tmp.hpp>
#include "Gui.hpp"

namespace inl::gui {

enum class eGuiListDirection
{
	VERTICAL,
	HORIZONTAL,
};

class GuiList : public Gui
{
public:
	GuiList(GuiEngine* guiEngine);
	GuiList(const GuiList& other) { *this = other; }

	// Important to implement in derived classes
	virtual GuiList* Clone() const override { return new GuiList(*this); }

	void SetDirection(eGuiListDirection dir);
	eGuiListDirection GetDirection() { return direction; }

protected:
	virtual Vector2f ArrangeChildren(const Vector2f& finalSize) override;

protected:
	eGuiListDirection direction;
};

inline GuiList::GuiList(GuiEngine* guiEngine)
:Gui(guiEngine), direction(eGuiListDirection::VERTICAL)
{
	SetBgColorForAllStates(GetBgIdleColor());
}

inline void GuiList::SetDirection(eGuiListDirection dir)
{
	direction = dir;
	bDirtyLayout = true;
}

inline Vector2f GuiList::ArrangeChildren(const Vector2f& finalSize)
{
	Vector2f pos = GetContentPos();
	Vector2f selfSize(0, 0);
	for (Gui* child : GetChildren())
	{
		Vector2f desiredSize = child->GetDesiredSize();
		if (direction == eGuiListDirection::VERTICAL)
		{
			Vector2f sizeUsed = child->Arrange(pos.x(), pos.y() + selfSize.y(), desiredSize);

			selfSize.y() += sizeUsed.y();
			selfSize.x() = std::max(selfSize.x(), sizeUsed.x());
		}
		else if (direction == eGuiListDirection::HORIZONTAL)
		{
			Vector2f sizeUsed = child->Arrange(pos.x() + selfSize.x(), pos.y(), desiredSize);

			selfSize.x() += sizeUsed.x();
			selfSize.y() = std::max(selfSize.y(), sizeUsed.y());
		}
	}

	return selfSize;
}

} // namespace inl::gui