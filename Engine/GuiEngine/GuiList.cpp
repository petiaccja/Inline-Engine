#include "GuiList.hpp"

using namespace inl::gui;

GuiList::GuiList(GuiEngine* guiEngine)
:GuiLayout(guiEngine), orientation(eGuiOrientation::VERTICAL)
{
	SetBgToColor(GetBgIdleColor());
}

void GuiList::SetOrientation(eGuiOrientation orientation)
{
	this->orientation = orientation;
	bLayoutNeedRefresh = true;
}

Vector2f GuiList::ArrangeChildren(const Vector2f& finalSize)
{
	Vector2f pos = GetContentPos();
	Vector2f selfSize(0, 0);
	for (Gui* child : GetItems())
	{
		Vector2f desiredSize = child->GetDesiredSize();
		if (orientation == eGuiOrientation::VERTICAL)
		{
			Vector2f sizeUsed = child->Arrange(pos.x(), pos.y() + selfSize.y(), desiredSize);

			selfSize.y() += sizeUsed.y();
			selfSize.x() = std::max(selfSize.x(), sizeUsed.x());
		}
		else if (orientation == eGuiOrientation::HORIZONTAL)
		{
			Vector2f sizeUsed = child->Arrange(pos.x() + selfSize.x(), pos.y(), desiredSize);

			selfSize.x() += sizeUsed.x();
			selfSize.y() = std::max(selfSize.y(), sizeUsed.y());
		}
	}

	return selfSize;
}