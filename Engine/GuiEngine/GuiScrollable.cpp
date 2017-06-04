#include "GuiScrollable.hpp"
#include "GuiButton.hpp"

using namespace inl::gui;

GuiScrollable::GuiScrollable(GuiEngine* guiEngine)
:GuiGrid(guiEngine), bVerScrollBarVisible(false), bHorScrollBarVisible(false)
{
	SetDimension(1, 2);

	Gui* contentCell = GetCell(0, 0);

	//Gui* verticalScrollCell = GetCell(1, 0);
	Gui* horizontalScrollCell = GetCell(0, 1);
	//Gui* emptyCell = GetCell(1, 1);
	
	contentCell->SetBgToColor(Color(45));
	//verticalScrollCell->SetBgToColor(Color(75));
	horizontalScrollCell->SetBgToColor(Color(75));
	//emptyCell->SetBgToColor(Color(75));

	// Content fill the space
	GetColumn(0)->StretchFillSpace(1.f);
	GetRow(0)->StretchFillSpace(1.f);

	// Scroll bars fixed size
	//GetColumn(1)->SetWidth(16);
	GetRow(1)->SetHeight(16);
	
	// Add scroll bars
	GuiButton* btn = horizontalScrollCell->AddGuiButton();
	btn->SetMargin(3);
	btn->SetBgToColor(Color(120), Color(200));
	btn->StretchVerFillParent();
	btn->SetWidth(250);
	//btn = verticalScrollCell->AddGuiButton();
	//btn->SetMargin(3);
	//btn->SetBgToColor(Color(120), Color(200));
	//btn->StretchHorFillParent();

	SetBgToColor(GetBgIdleColor());
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
	// The content cell we want to fill in
	Gui* contentCell = GetCell(0, 0);

	// Remove old content
	Gui* oldContent = contentCell->GetChild(0);
	if (oldContent)
		contentCell->RemoveGui(oldContent);

	// Add new content
	contentCell->AddGui(contentGui);

	//contentGui->onRectChangedClonable += [this, contentCell](Gui* self, RectF rect)
	//{
	//	RectF cellRect = contentCell->GetRect();
	//
	//	// Check if rect
	//	bool bShowHorScrollBar = rect.left < cellRect.left || rect.right > cellRect.right;
	//	bool bShowVerScrollBar = rect.top < cellRect.top || rect.bottom > cellRect.bottom;
	//
	//	if (bShowHorScrollBar && bShowVerScrollBar)
	//	{
	//		SetDimension(2, 2);
	//	}
	//	else if (bShowHorScrollBar)
	//	{
	//		SetDimension(1, 2);
	//	}
	//	else if (bShowVerScrollBar)
	//	{
	//		SetDimension(2, 1);
	//	}
	//	
	//	bVerScrollBarVisible = bShowVerScrollBar;
	//	bHorScrollBarVisible = bShowHorScrollBar;
	//};

	return contentGui;
}