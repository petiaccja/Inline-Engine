#include "GuiList.hpp"

using namespace inl::gui;

GuiList::GuiList(GuiEngine& guiEngine)
:GuiLayout(guiEngine), orientation(eGuiOrientation::VERTICAL)
{
	SetBgToColor(GetBgIdleColor());
	StretchFitToContent();
}

void GuiList::SetOrientation(eGuiOrientation orientation)
{
	this->orientation = orientation;
	bLayoutNeedRefresh = true;
}

Vec2 GuiList::ArrangeChildren()
{
	if (GetName() == L"__OPTIONS__")
	{
		int c = 0;
		c++;
	}

	Vec2 pos = GetContentPos();
	Vec2 selfSize(0, 0);
	for (Gui* child : GetItems())
	{
		Vec2 desiredSize = child->GetDesiredSize();
		if (orientation == eGuiOrientation::VERTICAL)
		{
			if (GetName() == L"__OPTIONS__")
			{
				int c = 0;
				c++;
			}

			Vec2 sizeUsed = child->Arrange(Vec2(pos.x, pos.y + selfSize.y), desiredSize);

			selfSize.y += sizeUsed.y;
			selfSize.x = std::max(selfSize.x, sizeUsed.x);
		}
		else if (orientation == eGuiOrientation::HORIZONTAL)
		{
			if (GetName() == L"__OPTIONS__")
			{
				int c = 0;
				c++;
			}

			Vec2 sizeUsed = child->Arrange(Vec2(pos.x + selfSize.x, pos.y), desiredSize);

			selfSize.x += sizeUsed.x;
			selfSize.y = std::max(selfSize.y, sizeUsed.y);
		}
	}

	if (GetName() == L"__OPTIONS__")
	{
		int c = 0;
		c++;
	}

	return selfSize;
}