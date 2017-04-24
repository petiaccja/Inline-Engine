#include "GuiSlider.hpp"
#include "GuiEngine.hpp"

using namespace inl::gui;

void GuiSplitter::AddItem(Gui* gui)
{
	if (items.size() > 0)
	{
		Gui* separator = AddGui();

		static bool bDragging = false;
		static Vector2f mousePosWhenPressed;
		static Vector2f leftItemOrigSize;
		static Vector2f rightItemOrigSize;
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
			
			Gui* leftItem = splitter->GetChild(separator->GetIndexInParent() - 1);
			Gui* rightItem = splitter->GetChild(separator->GetIndexInParent() + 1);

			leftItemOrigSize = leftItem->GetSize();
			rightItemOrigSize = rightItem->GetSize();

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

				leftItem->SetSize(Vector2f::Max(Vector2f(0,0), leftItemOrigSize + deltaMove));
				rightItem->SetSize(Vector2f::Max(Vector2f(0, 0), rightItemOrigSize - deltaMove));
			}
		};

		guiEngine->onMouseReleased += [this](CursorEvent& evt)
		{
			if (bDragging)
			{
				bDragging = false;
				guiEngine->SetCursorVisual(eCursorVisual::ARROW);
				guiEngine->DefreezeHover();
			}
		};

		separators.insert(separator);
		separator->SetSize(6, 0);
		separator->SetBgToColor(Color(120), Color(255));

		if (direction == eGuiDirection::HORIZONTAL)
			separator->StretchFillParent(false, true);
		else if (direction == eGuiDirection::VERTICAL)
			separator->StretchFillParent(true, false);
	}

	Add(gui);
	items.insert(gui);
}