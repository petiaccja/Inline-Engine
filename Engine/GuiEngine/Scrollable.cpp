#include "Scrollable.hpp"
#include "Button.hpp"

using namespace inl::gui;

ScrollableArea::ScrollableArea(GuiEngine& guiEngine)
:Grid(guiEngine), bVerScrollBarVisible(false), bHorScrollBarVisible(false)
{
	SetDimension(1, 2);

	Gui* contentCell = GetCell(0, 0);

	//Gui* verticalScrollCell = GetCell(1, 0);
	Gui* horizontalScrollCell = GetCell(0, 1);
	//Gui* emptyCell = GetCell(1, 1);
	
	contentCell->SetBgToColor(ColorI(25, 25, 25, 255));
	//verticalScrollCell->SetBgToColor(ColorI(75));
	horizontalScrollCell->SetBgToColor(ColorI(75, 75, 75, 255));
	//emptyCell->SetBgToColor(ColorI(75));

	// Content fill the space
	GetColumn(0)->StretchFillSpace(1.f);
	GetRow(0)->StretchFillSpace(1.f);

	// Scroll bars fixed size
	//GetColumn(1)->SetWidth(16);
	GetRow(1)->SetHeight(16);
	
	// Add scroll bars
	Button* btn = horizontalScrollCell->AddGui<Button>();
	btn->SetMargin(3);
	btn->SetBgToColor(ColorI(120, 120, 120, 255), ColorI(200, 200, 200, 255));
	btn->StretchVerFillParent();
	btn->SetWidth(250);
	//btn = verticalScrollCell->AddButton();
	//btn->SetMargin(3);
	//btn->SetBgToColor(ColorI(120), ColorI(200));
	//btn->StretchHorFillParent();

	SetBgToColor(GetBgIdleColor());
}

//void ScrollableArea::SetOrientation(eGuiOrientation orientation)
//{
//	this->orientation = orientation;
//	bLayoutNeedRefresh = true;
//}
//
//Vec2 ScrollableArea::ArrangeChildren(const Vec2& finalSize)
//{
//	Vec2 pos = GetContentPos();
//	Vec2 selfSize(0, 0);
//	for (Gui* child : GetChildren())
//	{
//		Vec2 desiredSize = child->GetDesiredSize();
//		if (orientation == eGuiOrientation::VERTICAL)
//		{
//			Vec2 sizeUsed = child->Arrange(pos.x, pos.y + selfSize.y, desiredSize);
//
//			selfSize.y += sizeUsed.y;
//			selfSize.x = std::max(selfSize.x, sizeUsed.x);
//		}
//		else if (orientation == eGuiOrientation::HORIZONTAL)
//		{
//			Vec2 sizeUsed = child->Arrange(pos.x + selfSize.x, pos.y, desiredSize);
//
//			selfSize.x += sizeUsed.x;
//			selfSize.y = std::max(selfSize.y, sizeUsed.y);
//		}
//	}
//
//	return selfSize;
//}

void ScrollableArea::SetContent(Gui* contentGui)
{
	// The content cell we want to fill in
	Gui* contentCell = GetCell(0, 0);

	// Remove old content
	Gui* oldContent = contentCell->GetChild(0);
	if (oldContent)
		contentCell->RemoveGui(oldContent);

	// Add new content
	contentCell->AddGui(contentGui);

	//contentGui->onRectChanged += [this, contentCell](Gui& self, GuiRectF rect)
	//{
	//	GuiRectF cellRect = contentCell->GetRect();
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
}