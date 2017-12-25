#include "GuiSlider.hpp"
#include "GuiEngine.hpp"

using namespace inl::gui;

GuiSplitter::GuiSplitter(GuiEngine& guiEngine)
:GuiLayout(guiEngine), orientation(eGuiOrientation::HORIZONTAL), separatorLength(10)
{
	SetBgToColor(GetBgIdleColor());
}

bool GuiSplitter::RemoveItem(Gui* gui)
{
	Gui* container = gui->GetParent();

	bool bGuiIdxInParent = container->GetIndexInParent();
	bool bRemoved = container->RemoveFromParent();

	std::vector<Gui*>& children = GetChildren();

	if (bRemoved && children.size() > 0)
	{	
		Gui* separator;
		if (bGuiIdxInParent == 0)						// First item removed, remove separator to the right
			separator = children[bGuiIdxInParent];
		else											// Non first item, so separators will be to the left
			separator = children[bGuiIdxInParent - 1]; 

		separators.erase(std::find(separators.begin(), separators.end(), separator));
		separator->RemoveFromParent();
	}

	items.erase(gui);

	return bRemoved;
}

Vec2 GuiSplitter::ArrangeChildren(const Vec2& finalSize)
{
	// The logic of the splitter arrangement is:
	// separators should always fill the whole area of splitter with preserving percentage !
	// So if splitter size is bigger than the needed space, percentage share the remained area between items ;)

	auto& items = GetItems();
	if (items.size() == 0)
		return GetSize();

	bool bVertical = GetOrientation() == eGuiOrientation::VERTICAL;

	float itemsLength = 0;
	for (Gui* child : items)
	{
		Gui* container = child->GetParent();

		if (bVertical)
			itemsLength += container->GetSize().y;
		else
			itemsLength += container->GetSize().x;
	}

	int separatorCount = items.size() - 1;
	float childrenLength = itemsLength + separatorCount * separatorLength;

	float freeSpace; // This value can be negative !
	if (bVertical)
		freeSpace = GetContentSize().y - childrenLength;
	else
		freeSpace = GetContentSize().x - childrenLength;


	for (int i = 0; i < items.size(); ++i)
	{
		Gui* container = items[i]->GetParent();

		// each item container know it's percentage [0,1] inside parent, so give them space proportionally
		Vec2 itemNormedSpacePercent = container->GetSize() / itemsLength;
		Vec2 itemFreeSpace = itemNormedSpacePercent * freeSpace;

		if (bVertical)
			container->SetSize(GetContentSize().x, container->GetSize().y + itemFreeSpace.y);
		else
			container->SetSize(container->GetSize().x + itemFreeSpace.x, GetContentSize().y);
	}

	// At this point all of our items are sized so they will proportionally fill the splitter control :)
	Vec2 pos = GetContentPos();
	Vec2 selfSize(0, 0);
	for (Gui* child : GetChildren())
	{
		Vec2 desiredSize = child->GetDesiredSize();
		if (bVertical)
		{
			Vec2 sizeUsed = child->Arrange(pos.x, pos.y + selfSize.y, desiredSize);

			selfSize.y += sizeUsed.y;
			selfSize.x = std::max(selfSize.x, sizeUsed.x);
		}
		else
		{
			Vec2 sizeUsed = child->Arrange(pos.x + selfSize.x, pos.y, desiredSize);

			selfSize.x += sizeUsed.x;
			selfSize.y = std::max(selfSize.y, sizeUsed.y);
		}
	}

	return selfSize;
}

void GuiSplitter::AddItem(Gui* item)
{
	if (items.size() > 0)
	{
		Gui* separator = AddGui();
		separators.push_back(separator);

		separator->OnCursorEntered += [](Gui& self_, CursorEvent& evt)
		{
			GuiSplitter& splitter = self_.GetParent()->As<GuiSplitter>();

			splitter.separatorSaved = &self_;

			if (splitter.GetOrientation() == eGuiOrientation::HORIZONTAL)
				splitter.guiEngine.SetCursorVisual(eCursorVisual::SIZEWE);
			if (splitter.GetOrientation() == eGuiOrientation::VERTICAL)
				splitter.guiEngine.SetCursorVisual(eCursorVisual::SIZENS);
		};

		separator->OnCursorLeft += [](Gui& self, CursorEvent& evt)
		{
			GuiSplitter& splitter = self.GetParent()->As<GuiSplitter>();

			if(!splitter.bDragging)
				self.guiEngine.SetCursorVisual(eCursorVisual::ARROW);
		};

		separator->OnCursorPressed += [](Gui& separator, CursorEvent& evt)
		{
			GuiSplitter& splitter = separator.GetParent()->As<GuiSplitter>();

			// We are starting to drag the separator, save the cursor pos
			splitter.bDragging = true;
			splitter.mousePosWhenPressed = evt.cursorPos;
			
			Gui* prevItem = splitter.GetChild(separator.GetIndexInParent() - 1);
			Gui* nextItem = splitter.GetChild(separator.GetIndexInParent() + 1);

			splitter.prevItemOrigSize = prevItem->GetSize();
			splitter.nextItemOrigSize = nextItem->GetSize();

			separator.guiEngine.FreezeHover();
		};

		guiEngine.OnCursorMoved += [this](CursorEvent& evt)
		{
			if (bDragging)
			{
				GuiSplitter& splitter = separatorSaved->GetParent()->As<GuiSplitter>();

				Vec2 deltaMouse = evt.cursorPos - mousePosWhenPressed;
				
				Gui* leftContainer = splitter.GetChild(separatorSaved->GetIndexInParent() - 1);
				Gui* rightContainer = splitter.GetChild(separatorSaved->GetIndexInParent() + 1);

				Vec2 deltaMove;
				if (splitter.GetOrientation() == eGuiOrientation::HORIZONTAL)
					deltaMove = Vec2(deltaMouse.x, 0);
				else if (splitter.GetOrientation() == eGuiOrientation::VERTICAL)
					deltaMove = Vec2(0, deltaMouse.y);
				
				// - TODO cursor goes outside of splitter gui, clamp size increase
				// - TODO separator collides with other separator, clamp
				Vec2 tmp0 = prevItemOrigSize + deltaMove;
				Vec2 tmp1 = nextItemOrigSize - deltaMove;
				leftContainer->SetSize(Vec2(std::max(0.f, tmp0.x), std::max(0.f, tmp0.y)));
				rightContainer->SetSize(Vec2(std::max(0.f, tmp1.x), std::max(0.f, tmp1.y)));

				leftContainer->RefreshLayout();
				rightContainer->RefreshLayout();
			}
		};

		guiEngine.OnCursorReleased += [this](CursorEvent& evt)
		{
			if (bDragging)
			{
				bDragging = false;
				guiEngine.DefreezeHover();
			}
		};

		separator->SetBgToColor(ColorI(135, 135, 135, 255), ColorI(220, 220, 220, 255));

		if (orientation == eGuiOrientation::HORIZONTAL)
		{
			separator->SetSize(separatorLength, 0);
			separator->StretchFillParent(false, true);
		}
		else
		{
			separator->SetSize(0, separatorLength);
			separator->StretchFillParent(true, false);
		}
	}

	// Gui Container wrapping our item, sizing and align policy will work relative to this container :)
	Gui* container = AddGui();
	container->DisableHover();
	container->AddGui(item);
	container->SetSize(item->GetSize());
	items.insert(item);
}

void GuiSplitter::SetOrientation(eGuiOrientation orientation)
{
	this->orientation = orientation;
	bLayoutNeedRefresh = true;
}

std::vector<Gui*> GuiSplitter::GetItems()
{
	std::vector<Gui*> result(items.size());

	int idx = 0;
	for (auto& gui : items)
		result[idx++] = gui;

	return result;
}