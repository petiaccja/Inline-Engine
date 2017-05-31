#include "GuiScrollable.hpp"
#include "GuiButton.hpp"

using namespace inl::gui;

GuiScrollable::GuiScrollable(GuiEngine* guiEngine)
:GuiGrid(guiEngine), orientation(eGuiOrientation::VERTICAL)
{
	SetDimension(2, 2);

	// Content
	GetColumn(0)->StretchFillSpace(1.f);
	GetRow(0)->StretchFillSpace(1.f);

	// Scroll bars
	GetColumn(1)->SetWidth(12);
	GetRow(1)->SetHeight(12);

	Gui* horizontalScrollCell = GetCell(1, 0);
	Gui* verticalScrollCell = GetCell(0, 1);

	GuiButton* btn = horizontalScrollCell->AddButton();
	btn->SetBgColorForAllStates(Color::RED);
	btn->StretchFillParent();

	btn = verticalScrollCell->AddButton();
	btn->SetBgColorForAllStates(Color::BLUE);
	btn->StretchFillParent();

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

Gui* GuiScrollable::SetContent(Gui* contentGui)
{
	//Gui* cell = GetRow(0)->GetCell(0);
	//if (cell)
	//	cell->Remove(cell->GetChild(0));
	//
	//cell->Add(contentGui);

	return contentGui;
}