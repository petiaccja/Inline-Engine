#pragma once
#include <BaseLibrary\Common_tmp.hpp>
#include "LayoutedGui.hpp"

namespace inl::gui {

class GuiList : public LayoutedGui
{
public:
	GuiList(GuiEngine* guiEngine);
	GuiList(const GuiList& other) { *this = other; }

	// Important to implement in derived classes
	virtual GuiList* Clone() const override { return new GuiList(*this); }

	virtual void AddItem(Gui* gui) override { Add(gui); }
	virtual bool RemoveItem(Gui* gui) override { return Remove(gui); }
	virtual std::vector<Gui*> GetItems() override { return GetChildren(); };

	void SetDirection(eGuiDirection dir);
	eGuiDirection GetDirection() { return direction; }

protected:
	virtual Vector2f ArrangeChildren(const Vector2f& finalSize) override;

protected:
	eGuiDirection direction;
};

inline GuiList::GuiList(GuiEngine* guiEngine)
:LayoutedGui(guiEngine), direction(eGuiDirection::VERTICAL)
{
	SetBgColorForAllStates(GetBgIdleColor());
}

inline void GuiList::SetDirection(eGuiDirection dir)
{
	direction = dir;
	bLayoutNeedRefresh = true;
}

inline Vector2f GuiList::ArrangeChildren(const Vector2f& finalSize)
{
	Vector2f pos = GetContentPos();
	Vector2f selfSize(0, 0);
	for (Gui* child : GetChildren())
	{
		Vector2f desiredSize = child->GetDesiredSize();
		if (direction == eGuiDirection::VERTICAL)
		{
			Vector2f sizeUsed = child->Arrange(pos.x(), pos.y() + selfSize.y(), desiredSize);

			selfSize.y() += sizeUsed.y();
			selfSize.x() = std::max(selfSize.x(), sizeUsed.x());
		}
		else if (direction == eGuiDirection::HORIZONTAL)
		{
			Vector2f sizeUsed = child->Arrange(pos.x() + selfSize.x(), pos.y(), desiredSize);

			selfSize.x() += sizeUsed.x();
			selfSize.y() = std::max(selfSize.y(), sizeUsed.y());
		}
	}

	return selfSize;
}

} // namespace inl::gui