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

void SetOrientation(eGuiDirection dir);
eGuiDirection GetOrientation() { return direction; }

protected:
	virtual Vector2f ArrangeChildren(const Vector2f& finalSize) override;

protected:
	std::unordered_set<Gui*> items;
	eGuiDirection direction;
	int separatorLength;
};

inline GuiSplitter::GuiSplitter(GuiEngine* guiEngine)
:LayoutedGui(guiEngine), direction(eGuiDirection::HORIZONTAL), separatorLength(8)
{
	SetBgColorForAllStates(GetBgIdleColor());
}

inline bool GuiSplitter::RemoveItem(Gui* gui)
{
	Gui* container = gui->GetParent();

	bool bGuiIdxInParent = container->GetIndexInParent();
	bool bRemoved = container->Remove();

	std::vector<Gui*>& children = GetChildren();

	if (bRemoved && children.size() > 0)
	{
		// First item removed, remove separator to the right
		if (bGuiIdxInParent == 0)
		{
			Gui* separator = children[bGuiIdxInParent];
			separator->Remove();
		}
		else // Non first item, so separators will be to the left
		{
			Gui* separator = children[bGuiIdxInParent - 1];
			separator->Remove();
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

inline void GuiSplitter::SetOrientation(eGuiDirection dir)
{
	direction = dir;
	bLayoutNeedRefresh = true;
}

inline Vector2f GuiSplitter::ArrangeChildren(const Vector2f& finalSize)
{
	// The logic of the splitter arrangement is:
	// separators should always fill the whole area of splitter with preserving percentage !
	// So if splitter size is bigger than the needed space, percentage share the remained area between items ;)

	auto& items = GetItems();
	if (items.size() == 0)
		return GetSize();

	//Vector2f allItemSize(0, 0);
	float itemsLength = 0;
	for (Gui* child : items)
	{
		Gui* container = child->GetParent();

		if (direction == eGuiDirection::VERTICAL)
			itemsLength += container->GetSizeY();
		else if (direction == eGuiDirection::HORIZONTAL)
			itemsLength += container->GetSizeX();
	}

	int separatorCount = items.size() - 1;
	float childrenLength = itemsLength + separatorCount * separatorLength;

	float freeSpace; // This value can be negative !
	if (direction == eGuiDirection::VERTICAL)
		freeSpace = GetContentSizeY() - childrenLength;
	else if (direction == eGuiDirection::HORIZONTAL)
		freeSpace = GetContentSizeX() - childrenLength;


	for (int i = 0; i < items.size(); ++i)
	{
		Gui* container = items[i]->GetParent();

		// each item container know it's percentage [0,1] inside parent, so give them space proportionally
		Vector2f itemNormedSpacePercent = container->GetSize() / itemsLength;
		Vector2f itemFreeSpace = itemNormedSpacePercent * freeSpace;

		if (direction == eGuiDirection::VERTICAL)
			container->SetSize(GetContentSizeX(), container->GetSizeY() + itemFreeSpace.y());
		else if (direction == eGuiDirection::HORIZONTAL)
			container->SetSize(container->GetSizeX() + itemFreeSpace.x(), GetContentSizeY());
	}

	//for (Gui* child : items)
	//{
	//	float itemFreeSpace = 0.f;
	//
	//	if (child == items[items.size() - 1])
	//		itemFreeSpace = freeSpace;
	//
	//	Gui* container = child->GetParent();
	//
	//	if (direction == eGuiDirection::VERTICAL)
	//		container->SetSize(GetSizeX(), container->GetSizeY() + itemFreeSpace);
	//	else if (direction == eGuiDirection::HORIZONTAL)
	//		container->SetSize(container->GetSizeX() + itemFreeSpace, GetSizeY());
	//}

	// At this point all of our items are sized so they will proportionally fill the splitter control :)
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