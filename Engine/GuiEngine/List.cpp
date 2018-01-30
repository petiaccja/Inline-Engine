#include "List.hpp"

namespace inl::gui {

ListView::ListView(GuiEngine* guiEngine)
	:Layout(guiEngine), orientation(eGuiOrientation::VERTICAL)
{
	SetBgToColor(GetBgIdleColor());
	StretchFitToContent();
}

void ListView::SetOrientation(eGuiOrientation orientation)
{
	this->orientation = orientation;
	bLayoutNeedRefresh = true;
}

Vec2 ListView::ArrangeChildren()
{
	Vec2 pos = GetContentPos();
	Vec2 selfSize(0, 0);
	for (Gui* child : GetItems())
	{
		Vec2 desiredSize = child->GetDesiredSize();
		if (orientation == eGuiOrientation::VERTICAL)
		{
			Vec2 sizeUsed = child->Arrange(Vec2(pos.x, pos.y + selfSize.y), desiredSize);

			selfSize.y += sizeUsed.y;
			selfSize.x = std::max(selfSize.x, sizeUsed.x);
		}
		else if (orientation == eGuiOrientation::HORIZONTAL)
		{
			Vec2 sizeUsed = child->Arrange(Vec2(pos.x + selfSize.x, pos.y), desiredSize);

			selfSize.x += sizeUsed.x;
			selfSize.y = std::max(selfSize.y, sizeUsed.y);
		}
	}

	return selfSize;
}

}