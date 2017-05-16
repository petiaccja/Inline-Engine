#include "GuiScrollable.hpp"

using namespace inl::gui;

GuiScrollable::GuiScrollable(GuiEngine* guiEngine)
:GuiLayouted(guiEngine), orientation(eGuiOrientation::VERTICAL), contentGui(nullptr)
{
	SetBgColorForAllStates(GetBgIdleColor());
}

//void GuiScrollable::SetOrientation(eGuiOrientation orientation)
//{
//	this->orientation = orientation;
//	bLayoutNeedRefresh = true;
//}
//
//Vector2f GuiScrollable::ArrangeChildren(const Vector2f& finalSize)
//{
//	Vector2f pos = GetContentPos();
//	Vector2f selfSize(0, 0);
//	for (Gui* child : GetChildren())
//	{
//		Vector2f desiredSize = child->GetDesiredSize();
//		if (orientation == eGuiOrientation::VERTICAL)
//		{
//			Vector2f sizeUsed = child->Arrange(pos.x(), pos.y() + selfSize.y(), desiredSize);
//
//			selfSize.y() += sizeUsed.y();
//			selfSize.x() = std::max(selfSize.x(), sizeUsed.x());
//		}
//		else if (orientation == eGuiOrientation::HORIZONTAL)
//		{
//			Vector2f sizeUsed = child->Arrange(pos.x() + selfSize.x(), pos.y(), desiredSize);
//
//			selfSize.x() += sizeUsed.x();
//			selfSize.y() = std::max(selfSize.y(), sizeUsed.y());
//		}
//	}
//
//	return selfSize;
//}

void GuiScrollable::SetContent(Gui* contentGui)
{
	if (this->contentGui)
		this->contentGui->RemoveFromParent();

	this->contentGui = contentGui;
}