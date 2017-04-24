#pragma once
#include <BaseLibrary\Common_tmp.hpp>
#include "LayoutedGui.hpp"
#include <unordered_set>

namespace inl::gui {

class GuiSplitter : public LayoutedGui
{
public:
	GuiSplitter(GuiEngine* guiEngine);
	GuiSplitter(const GuiSplitter& other) { *this = other; }

	// Important to implement in derived classes
	virtual GuiSplitter* Clone() const override { return new GuiSplitter(*this); }

	virtual void AddItem(Gui* gui) override;
	virtual bool RemoveItem(Gui* gui) override;
	virtual std::vector<Gui*> GetItems() override;

	void SetDirection(eGuiDirection dir);
	eGuiDirection GetDirection() { return direction; }

protected:
	virtual Vector2f ArrangeChildren(const Vector2f& finalSize) override;

protected:
	std::unordered_set<Gui*> items;
	std::unordered_set<Gui*> separators;
	eGuiDirection direction;
};

inline GuiSplitter::GuiSplitter(GuiEngine* guiEngine)
:LayoutedGui(guiEngine), direction(eGuiDirection::HORIZONTAL)
{
	SetBgColorForAllStates(GetBgIdleColor());
}

inline bool GuiSplitter::RemoveItem(Gui* gui)
{
	bool bGuiIdxInParent = gui->GetIndexInParent();
	bool bRemoved = gui->Remove();

	std::vector<Gui*>& children = GetChildren();

	if (bRemoved && children.size() > 0)
	{
		// First item removed, remove separator to the right
		if (bGuiIdxInParent == 0)
		{
			Gui* separator = children[bGuiIdxInParent];
			separator->Remove();
			separators.erase(separator);
		}
		else // Non first item, so separators will be left
		{
			Gui* separator = children[bGuiIdxInParent - 1];
			separator->Remove();
			separators.erase(separator);
		}
	}

	items.erase(gui);

	return bRemoved;
}

inline std::vector<Gui*> GuiSplitter::GetItems()
{
	std::vector<Gui*> result(items.size());

	int idx = 0;
	for (auto& gui : items)
		result[idx++] = gui;

	return result;
}

inline void GuiSplitter::SetDirection(eGuiDirection dir)
{
	direction = dir;
	bLayoutNeedRefresh = true;
}

inline Vector2f GuiSplitter::ArrangeChildren(const Vector2f& finalSize)
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