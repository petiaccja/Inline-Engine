#include "GuiEngine.hpp"

using namespace inl::gui;

GuiEngine::GuiEngine(gxeng::GraphicsEngine* graphicsEngine, Window* targetWindow)
{
	// TEMPORARY
	gdiGraphics = nullptr;
	hdc = nullptr;
	memHDC = nullptr;

	bOperSysDragging = false;
	this->targetWindow = targetWindow;
	this->graphicsEngine = graphicsEngine;
	bHoverFreezed = false;
	hoveredGui = nullptr;
	activeContextMenu = nullptr;
	postProcessLayer = CreateLayer();
	cursorVisual = eCursorVisual::ARROW;

	// Initialize GDI+ for gui rendering
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	SetResolution(targetWindow->GetClientSize());
	targetWindow->onClientSizeChanged += [this](Vec2u size)
	{
		SetResolution(size);
	};

	// Propagate mousePress
	targetWindow->onMousePressed += [&, targetWindow](WindowEvent& event)
	{
		CursorEvent eventData;
		eventData.cursorPos = event.clientMousePos;
		onMousePressed(eventData);

		mousePosWhenPress = event.clientMousePos;

		Vec2 pixelCenterCursorPos = event.clientMousePos + Vec2(0.5, 0.5); // Important, make cursorPos pointing to the center of the pixel ! Making children gui - s non overlappable at edges

		if (hoveredGui)
		{
			hoveredGui->TraverseTowardParents([&](Gui* control)
			{
				if (control->GetVisiblePaddingRect().IsPointInside(pixelCenterCursorPos))
				{
					control->onMousePressed(eventData);
					control->onMousePressedClonable(control, eventData);
				}
			});
		}

		if (activeContextMenu)
			activeContextMenu->RemoveFromParent();
	};

	// Propagate mouseRelease
	targetWindow->onMouseReleased += [&](WindowEvent& event)
	{
		CursorEvent eventData;
		eventData.cursorPos = event.clientMousePos;
		onMouseReleased(eventData);

		// Mouse click
		bool bClick = mousePosWhenPress == event.clientMousePos;

		if (bClick)
			onMouseClicked(eventData);

		if (activeContextMenu)
			activeContextMenu->RemoveFromParent();

		Vec2 pixelCenterCursorPos = event.clientMousePos + Vec2(0.5, 0.5); // Important, make cursorPos pointing to the center of the pixel ! Making children gui - s non overlappable at edges

		if (hoveredGui)
		{
			// Control Mouse release
			hoveredGui->TraverseTowardParents([&](Gui* control)
			{
				if (control->GetVisiblePaddingRect().IsPointInside(pixelCenterCursorPos))
				{
					control->onMouseReleased(eventData);
					control->onMouseReleasedClonable(control, eventData);
				}
			});

			// Control Mouse click
			if (bClick)
			{
				hoveredGui->TraverseTowardParents([&](Gui* control)
				{
					if (control->GetVisiblePaddingRect().IsPointInside(pixelCenterCursorPos))
					{
						control->onMouseClicked(eventData);
						control->onMouseClickedClonable(control, eventData);

						if (event.mouseBtn == eMouseBtn::RIGHT)
						{
							if (control->GetContextMenu())
							{
								// Add to the top most layer
								if (layers.size() > 0)
								{
									activeContextMenu = control->GetContextMenu();

									if (activeContextMenu)
									{
										postProcessLayer->AddGui(activeContextMenu);

										RectF rect = activeContextMenu->GetRect();
										rect.left = event.clientMousePos.x;
										rect.top = event.clientMousePos.y;
										activeContextMenu->SetRect(rect);
									}
								}
							}
						}
					}
				});
			}
		}
	};

	// Propagate onMouseMove
	targetWindow->onMouseMoved += [&](WindowEvent& event)
	{
		CursorEvent eventData;
		eventData.cursorPos = event.clientMousePos;
		eventData.mouseDelta = event.mouseDelta;

		onMouseMoved(eventData);

		Vec2 pixelCenterCursorPos = event.clientMousePos + Vec2(0.5, 0.5); // Important, make cursorPos pointing to the center of the pixel ! Making children gui - s non overlappable at edges
		if (hoveredGui)
		{
			hoveredGui->TraverseTowardParents([&](Gui* control)
			{
				if (control->GetVisiblePaddingRect().IsPointInside(pixelCenterCursorPos))
				{
					control->onMouseMoved(eventData);
					control->onMouseMovedClonable(control, eventData);
				}
			});
		}
	};

	targetWindow->onDragEntered += [this](DragData& data)
	{
		bOperSysDragging = true;

		dragData = data;

		Gui* hoveredGui = GetHoveredGui();

		if (hoveredGui)
			hoveredGui->onOperSysDragEntered(data);
	};

	targetWindow->onDragLeaved += [this](DragData& data)
	{
		bOperSysDragging = false;
	};

	targetWindow->onDropped += [this](DragData& data)
	{
		bOperSysDragging = false;

		Gui* hoveredGui = GetHoveredGui();

		if(hoveredGui)
			hoveredGui->onOperSysDropped(data);
	};

	targetWindow->onClientSizeChanged += [this](Vec2u size)
	{
		for (auto& layer : GetLayers())
			layer->SetSize(Vec2(size.x, size.y));
	};
}

GuiEngine::~GuiEngine()
{
	for (Gui* gui : guis)
		delete gui;

	layers.clear();

	DeleteObject(memBitmap);
	DeleteDC(hdc);
	DeleteDC(memHDC);
}

void GuiEngine::SetResolution(Vec2u& size)
{
	DeleteObject(memBitmap);
	DeleteDC(hdc);
	DeleteDC(memHDC);

	Gdiplus::Graphics* graphics = Gdiplus::Graphics::FromHWND((HWND)targetWindow->GetHandle());
	graphics->SetSmoothingMode(Gdiplus::SmoothingModeDefault);

	hdc = graphics->GetHDC();

	memHDC = CreateCompatibleDC(hdc);
	memBitmap = CreateCompatibleBitmap(hdc, size.x, size.y);

	SelectObject(memHDC, memBitmap);

	gdiGraphics = Gdiplus::Graphics::FromHDC(memHDC);
}

GuiLayer* GuiEngine::AddLayer()
{
	GuiLayer* layer = CreateLayer();
	layers.push_back(layer);
	return layer;
}

GuiLayer* GuiEngine::CreateLayer()
{
	return new GuiLayer(this);
}

void GuiEngine::Update(float deltaTime)
{
	// Let's hint the window to repaint itself
	InvalidateRect((HWND)targetWindow->GetHandle(), NULL, false);

	// Calculate clipping rect for all gui controls
	std::function<void(Gui* control, RectF& clipRect)> traverseControls;
	traverseControls = [&](Gui* control, RectF& clipRect)
	{
		control->SetVisibleRect(clipRect);

		// Control the clipping rect of the children controls
		RectF rect;
		if (control->IsChildrenClipEnabled())
			rect = control->GetPaddingRect();
		else
			rect = RectF(-FLT_MAX * 0.5, FLT_MAX, -FLT_MAX * 0.5, FLT_MAX);

		RectF newClipRect = RectF::Intersect(clipRect, rect);

		for (Gui* child : control->GetChildren())
			traverseControls(child, newClipRect);
	};

	for (GuiLayer* layer : GetLayers())
	{
		RectF clipRect(-FLT_MAX * 0.5, FLT_MAX, -FLT_MAX * 0.5, FLT_MAX);
		traverseControls(layer, clipRect);
	}


	// Call Update callback for all gui controls
	TraverseGuiControls([=](Gui* control)
	{
		control->onUpdate(deltaTime);
		control->onUpdateClonable(control, deltaTime);
	});

	// Search for hovered control, handle MouseLeaved, MouseEntered, MouseHovering
	if (!IsHoverFreezed())
	{
		Vec2 cursorPos = targetWindow->GetClientCursorPos();

		CursorEvent eventData(cursorPos);

		// Search hovered control to fire event on them
		cursorPos += Vec2(0.5f, 0.5f); // Important, make cursorPos pointing to the center of the pixel ! Making children gui - s non overlappable at edges
		Gui* newHoveredControl = nullptr;
		TraverseGuiControls([&](Gui* control)
		{
			if (!control->IsLayer() && control->IsHoverable() && control->GetVisiblePaddingRect().IsPointInside(cursorPos))
				newHoveredControl = control;
		});

		if (newHoveredControl != hoveredGui)
		{
			// Cursor Leave
			if (hoveredGui)
			{
				hoveredGui->TraverseTowardParents([&](Gui* control)
				{
					control->onMouseLeaved(eventData);
					control->onMouseLeavedClonable(control, eventData);
				});
			}

			// Cursor Enter
			if (newHoveredControl)
			{
				newHoveredControl->TraverseTowardParents([&](Gui* control)
				{
					if (control->GetVisiblePaddingRect().IsPointInside(cursorPos))
					{
						control->onMouseEntered(eventData);
						control->onMouseEnteredClonable(control, eventData);
					}
				});
			}
		}
		else
		{
			// Cursor Hover
			if (hoveredGui)
			{
				hoveredGui->TraverseTowardParents([&](Gui* control)
				{
					if (control->GetVisiblePaddingRect().IsPointInside(cursorPos) && control->IsHoverable())
					{
						control->onMouseHovering(eventData);
						control->onMouseHoveringClonable(control, eventData);
					}
				});
			}
		}
		hoveredGui = newHoveredControl;
	}

	if (bOperSysDragging)
	{
		Gui* hoveredGui = GetHoveredGui();
		
		if (hoveredGui)
			hoveredGui->onOperSysDragHovering(dragData);
	}
}

void GuiEngine::Render()
{
	SelectObject(memHDC, memBitmap);

	std::function<void(Gui* control)> traverseControls;
	traverseControls = [&](Gui* control)
	{
		control->onPaint(gdiGraphics);
		control->onPaintClonable(control, gdiGraphics);

		for (Gui* child : control->GetChildren())
			traverseControls(child);
	};

	for (GuiLayer* layer : GetLayers())
		traverseControls(layer);

	// Present
	BitBlt(hdc, 0, 0, targetWindow->GetClientWidth(), targetWindow->GetClientHeight(), memHDC, 0, 0, SRCCOPY);
}

void GuiEngine::TraverseGuiControls(const std::function<void(Gui*)>& fn)
{
	std::function<void(Gui* control, const std::function<void(Gui* control)>& fn)> traverseControls;
	traverseControls = [&](Gui* control, const std::function<void(Gui* control)>& fn)
	{
		fn(control);

		for (Gui* child : control->GetChildren())
			traverseControls(child, fn);
	};

	auto allLayers = layers;
	allLayers.push_back(postProcessLayer);

	for (GuiLayer* layer : allLayers)
	{
		traverseControls(layer, [&](Gui* control)
		{
			fn(control);
		});
	}
}

void GuiEngine::SetCursorVisual(eCursorVisual cursorVisual)
{
	this->cursorVisual = cursorVisual;
	Sys::SetCursorVisual(cursorVisual, targetWindow->GetHandle());
}

std::vector<GuiLayer*> GuiEngine::GetLayers()
{
	std::vector<GuiLayer*> result = layers;
	result.push_back(postProcessLayer);

	return result;
}