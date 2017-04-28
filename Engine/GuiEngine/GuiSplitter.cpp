#include "GuiSlider.hpp"
#include "GuiEngine.hpp"

using namespace inl::gui;

void GuiSplitter::AddItem(Gui* gui)
{
	if (items.size() > 0)
	{
		Gui* separator = AddGui();

		// TODO MOVE ELSWHERE
		if (GetDirection() == eGuiDirection::HORIZONTAL)
			separator->SetBorder(1, 0, 1, 0, Color(0));
		if (GetDirection() == eGuiDirection::VERTICAL)
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
			if (splitter->GetDirection() == eGuiDirection::HORIZONTAL)
				splitter->guiEngine->SetCursorVisual(eCursorVisual::SIZEWE);
			if (splitter->GetDirection() == eGuiDirection::VERTICAL)
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
				if (splitter->GetDirection() == eGuiDirection::HORIZONTAL)
					deltaMove = Vector2f(deltaMouse.x(), 0);
				else if (splitter->GetDirection() == eGuiDirection::VERTICAL)
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

		if (direction == eGuiDirection::HORIZONTAL)
		{
			separator->SetSize(separatorLength, 0);
			separator->StretchFillParent(false, true);
		}
		else if (direction == eGuiDirection::VERTICAL)
		{
			separator->SetSize(0, separatorLength);
			separator->StretchFillParent(true, false);
		}
	}

	// Gui Container wrapping our item, sizing and align policy will work relative to this container :)
	Gui* container = AddGui();
	//container->StretchFitToChildren();
	container->DisableHover();
	container->Add(gui);
	//container->SetBorder(2, Color::RED);
	items.insert(gui);
}