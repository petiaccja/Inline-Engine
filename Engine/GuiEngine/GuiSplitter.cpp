#include "GuiSlider.hpp"
#include "GuiEngine.hpp"

using namespace inl::gui;

GuiSplitter::GuiSplitter(GuiEngine* guiEngine)
:LayoutedGui(guiEngine), orientation(eGuiOrientation::HORIZONTAL), separatorLength(8)
{
	SetBgColorForAllStates(GetBgIdleColor());
}

bool GuiSplitter::RemoveItem(Gui* gui)
{
	Gui* container = gui->GetParent();

	bool bGuiIdxInParent = container->GetIndexInParent();
	bool bRemoved = container->RemoveFromParent();

	std::vector<Gui*>& children = GetChildren();

	if (bRemoved && children.size() > 0)
	{
		// First item removed, remove separator to the right
		if (bGuiIdxInParent == 0)
		{
			Gui* separator = children[bGuiIdxInParent];
			separator->RemoveFromParent();
		}
		else // Non first item, so separators will be to the left
		{
			Gui* separator = children[bGuiIdxInParent - 1];
			separator->RemoveFromParent();
		}
	}

	items.erase(gui);

	return bRemoved;
}

std::vector<Gui*> GuiSplitter::GetItems()
{
	std::vector<Gui*> result(items.size());

	int idx = 0;
	for (auto& gui : items)
		result[idx++] = gui;

	return result;
}

void GuiSplitter::SetOrientation(eGuiOrientation orientation)
{
	this->orientation = orientation;
	bLayoutNeedRefresh = true;
}

Vector2f GuiSplitter::ArrangeChildren(const Vector2f& finalSize)
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
			itemsLength += container->GetSizeY();
		else
			itemsLength += container->GetSizeX();
	}

	int separatorCount = items.size() - 1;
	float childrenLength = itemsLength + separatorCount * separatorLength;

	float freeSpace; // This value can be negative !
	if (bVertical)
		freeSpace = GetContentSizeY() - childrenLength;
	else
		freeSpace = GetContentSizeX() - childrenLength;


	for (int i = 0; i < items.size(); ++i)
	{
		Gui* container = items[i]->GetParent();

		// each item container know it's percentage [0,1] inside parent, so give them space proportionally
		Vector2f itemNormedSpacePercent = container->GetSize() / itemsLength;
		Vector2f itemFreeSpace = itemNormedSpacePercent * freeSpace;

		if (bVertical)
			container->SetSize(GetContentSizeX(), container->GetSizeY() + itemFreeSpace.y());
		else
			container->SetSize(container->GetSizeX() + itemFreeSpace.x(), GetContentSizeY());
	}

	// At this point all of our items are sized so they will proportionally fill the splitter control :)
	Vector2f pos = GetContentPos();
	Vector2f selfSize(0, 0);
	for (Gui* child : GetChildren())
	{
		Vector2f desiredSize = child->GetDesiredSize();
		if (bVertical)
		{
			Vector2f sizeUsed = child->Arrange(pos.x(), pos.y() + selfSize.y(), desiredSize);

			selfSize.y() += sizeUsed.y();
			selfSize.x() = std::max(selfSize.x(), sizeUsed.x());
		}
		else
		{
			Vector2f sizeUsed = child->Arrange(pos.x() + selfSize.x(), pos.y(), desiredSize);

			selfSize.x() += sizeUsed.x();
			selfSize.y() = std::max(selfSize.y(), sizeUsed.y());
		}
	}

	return selfSize;
}

void GuiSplitter::AddItem(Gui* gui)
{
	if (items.size() > 0)
	{
		Gui* separator = AddGui();

		// TODO MOVE ELSWHERE
		if (GetOrientation() == eGuiOrientation::HORIZONTAL)
			separator->SetBorder(1, 0, 1, 0, Color(0));
		if (GetOrientation() == eGuiOrientation::VERTICAL)
			separator->SetBorder(0, 1, 0, 1, Color(0));

		static bool bDragging = false;
		static Vector2f mousePosWhenPressed;
		static Vector2f prevItemOrigSize;
		static Vector2f nextItemOrigSize;
		static Gui* separatorr;
		separator->onMouseEnteredClonable += [](Gui* _self, CursorEvent& evt)
		{
			GuiSplitter* splitter = _self->GetParent()->AsSplitter();
			separatorr = _self;
			if (splitter->GetOrientation() == eGuiOrientation::HORIZONTAL)
				splitter->guiEngine->SetCursorVisual(eCursorVisual::SIZEWE);
			if (splitter->GetOrientation() == eGuiOrientation::VERTICAL)
				splitter->guiEngine->SetCursorVisual(eCursorVisual::SIZENS);
		};

		separator->onMouseLeavedClonable += [](Gui* self, CursorEvent& evt)
		{
			if(!bDragging)
				self->guiEngine->SetCursorVisual(eCursorVisual::ARROW);
		};

		separator->onMousePressedClonable += [](Gui* separator, CursorEvent& evt)
		{
			GuiSplitter* splitter = separator->GetParent()->AsSplitter();

			// We are starting to drag the separator, save the cursor pos
			bDragging = true;
			mousePosWhenPressed = evt.cursorPos;
			
			Gui* prevItem = splitter->GetChild(separator->GetIndexInParent() - 1);
			Gui* nextItem = splitter->GetChild(separator->GetIndexInParent() + 1);

			prevItemOrigSize = prevItem->GetSize();
			nextItemOrigSize = nextItem->GetSize();

			separator->guiEngine->FreezeHover();
		};

		guiEngine->onMouseMoved += [](CursorEvent& evt)
		{
			if (bDragging)
			{
				GuiSplitter* splitter = separatorr->GetParent()->AsSplitter();

				Vector2f deltaMouse = evt.cursorPos - mousePosWhenPressed;
				
				Gui* leftItem = splitter->GetChild(separatorr->GetIndexInParent() - 1);
				Gui* rightItem = splitter->GetChild(separatorr->GetIndexInParent() + 1);

				Vector2f deltaMove;
				if (splitter->GetOrientation() == eGuiOrientation::HORIZONTAL)
					deltaMove = Vector2f(deltaMouse.x(), 0);
				else if (splitter->GetOrientation() == eGuiOrientation::VERTICAL)
					deltaMove = Vector2f(0, deltaMouse.y());
				
				// - TODO cursor goes outside of splitter gui, clamp size increase
				// - TODO separator collides with other separator, clamp

				leftItem->SetSize(Vector2f::Max(Vector2f(0,0), prevItemOrigSize + deltaMove));
				rightItem->SetSize(Vector2f::Max(Vector2f(0, 0), nextItemOrigSize - deltaMove));

				// TODO Enélkül flick meg késés, ez így nem normális TODO !!!!!!!!
				leftItem->RefreshLayout();
				rightItem->RefreshLayout();
			}
		};

		guiEngine->onMouseReleased += [this](CursorEvent& evt)
		{
			if (bDragging)
			{
				bDragging = false;
				guiEngine->DefreezeHover();
			}
		};

		
		separator->SetBgToColor(Color(120), Color(255));

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
	container->Add(gui);
	items.insert(gui);
}