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
	targetWindow->OnResize+= [this](ResizeEvent& e)
	{
		SetResolution(e.clientSize);

		for (auto& layer : GetLayers())
			layer->SetSize(e.clientSize);
	};

	//Propagate mousePress
	targetWindow->OnMouseButton += [&](MouseButtonEvent e)
	{
		if (e.state == eKeyState::DOWN)
		{
			CursorEvent eventData;
			eventData.cursorPos = Vec2(e.x, e.y);
			OnCursorPress(eventData);

			mousePosWhenPress = eventData.cursorPos;

			Vec2 pixelCenterCursorPos = eventData.cursorPos + Vec2(0.5, 0.5); // Important, make cursorPos pointing to the center of the pixel ! Making children gui - s non overlappable at edges

			if (hoveredGui)
			{
				hoveredGui->TraverseTowardParents([&](Gui& control)
				{
					if (control.GetVisiblePaddingRect().IsPointInside(pixelCenterCursorPos))
						control.OnCursorPress(control, eventData);
				});
			}

			if (activeContextMenu)
				activeContextMenu->RemoveFromParent();
		}
	};


	// Propagate mouseRelease
	targetWindow->OnMouseButton += [&](MouseButtonEvent e)
	{
		if (e.state == eKeyState::UP)
		{
			CursorEvent eventData;
			eventData.cursorPos = Vec2(e.x, e.y);
			eventData.mouseButton = e.button;
			OnCursorRelease(eventData);

			// Mouse click
			bool bClick = mousePosWhenPress == eventData.cursorPos;

			if (bClick)
				OnCursorClick(eventData);

			if (activeContextMenu)
				activeContextMenu->RemoveFromParent();

			Vec2 pixelCenterCursorPos = eventData.cursorPos + Vec2(0.5, 0.5); // Important, make cursorPos pointing to the center of the pixel ! Making children gui - s non overlappable at edges

			if (hoveredGui)
			{
				// Control Mouse release
				hoveredGui->TraverseTowardParents([&](Gui& control)
				{
					if (control.GetVisiblePaddingRect().IsPointInside(pixelCenterCursorPos))
						control.OnCursorRelease(control, eventData);
				});

				// Control Mouse click
				if (bClick)
				{
					hoveredGui->TraverseTowardParents([&](Gui& control)
					{
						if (control.GetVisiblePaddingRect().IsPointInside(pixelCenterCursorPos))
						{
							control.OnCursorClick(control, eventData);

							if (e.button == eMouseButton::RIGHT)
							{
								if (control.GetContextMenu())
								{
									// Add to the top most layer
									if (layers.size() > 0)
									{
										activeContextMenu = control.GetContextMenu();

										if (activeContextMenu)
										{
											postProcessLayer->AddGui(activeContextMenu);

											GuiRectF rect = activeContextMenu->GetRect();
											rect.left = eventData.cursorPos.x;
											rect.top = eventData.cursorPos.x;
											activeContextMenu->SetRect(rect);
										}
									}
								}
							}
						}
					});
				}
			}
		}
	};


	// Propagate onCursorMove
	targetWindow->OnMouseMove += [&](MouseMoveEvent e)
	{
		CursorEvent eventData;
		eventData.cursorPos = Vec2(e.absx, e.absy);
		eventData.cursorDelta = Vec2(e.relx, e.rely);

		OnCursorMove(eventData);

		Vec2 pixelCenterCursorPos = eventData.cursorPos + Vec2(0.5, 0.5); // Important, make cursorPos pointing to the center of the pixel ! Making children gui - s non overlappable at edges
		if (hoveredGui)
		{
			hoveredGui->TraverseTowardParents([&](Gui& control)
			{
				if (control.GetVisiblePaddingRect().IsPointInside(pixelCenterCursorPos))
				{
					control.OnCursorMove(control, eventData);
				}
			});
		}
	};

	targetWindow->OnDropEnter += [this](DragDropEvent e)
	{
		bOperSysDragging = true;

		lastDropEvent = e;

		Gui* hoveredGui = GetHoveredGui();

		if (hoveredGui)
			hoveredGui->OnOperSysDragEnter(*hoveredGui, e);
	};

	targetWindow->OnDropLeave += [this](DragDropEvent e)
	{
		bOperSysDragging = false;
	};

	targetWindow->OnDrop += [this](DragDropEvent e)
	{
		bOperSysDragging = false;

		Gui* hoveredGui = GetHoveredGui();

		if(hoveredGui)
			hoveredGui->OnOperSysDrop(*hoveredGui, e);
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

	Gdiplus::Graphics* graphics = Gdiplus::Graphics::FromHWND((HWND)targetWindow->GetNativeHandle());
	graphics->SetSmoothingMode(Gdiplus::SmoothingModeDefault);

	hdc = graphics->GetHDC();

	memHDC = CreateCompatibleDC(hdc);
	memBitmap = CreateCompatibleBitmap(hdc, size.x, size.y);

	SelectObject(memHDC, memBitmap);

	gdiGraphics = Gdiplus::Graphics::FromHDC(memHDC);
}

Layer* GuiEngine::AddLayer()
{
	Layer* layer = CreateLayer();
	layers.push_back(layer);
	return layer;
}

Layer* GuiEngine::CreateLayer()
{
	return new Layer(*this);
}

void GuiEngine::Update(float deltaTime)
{
	// Let's hint the window to repaint itself
	InvalidateRect((HWND)targetWindow->GetNativeHandle(), NULL, false);

	GuiRectF clipRect(std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max(), std::numeric_limits<int16_t>::max(), std::numeric_limits<int16_t>::min());

	// Calculate clipping rect for all gui controls
	std::function<void(Gui* control, GuiRectF& clipRect)> traverseControls;
	traverseControls = [&](Gui* control, GuiRectF& clipRect)
	{
		control->SetVisibleRect(clipRect);

		// Control the clipping rect of the children controls
		GuiRectF rect;
		if (control->IsChildrenClipEnabled())
			rect = control->GetPaddingRect();
		else
			rect = clipRect;

		GuiRectF newClipRect = GuiRectF::Intersection(clipRect, rect);

		for (Gui* child : control->GetChildren())
			traverseControls(child, newClipRect);
	};

	for (Layer* layer : GetLayers())
		traverseControls(layer, clipRect);


	// Call Update callback for all gui controls
	TraverseGuiControls([&](Gui& control)
	{
		UpdateEvent updateEvent;
		updateEvent.deltaTime = deltaTime;
		control.OnUpdate(control, updateEvent);
	});

	// Search for hovered control, handle MouseLeft, MouseEntered, MouseHovering
	if (!IsHoverFreezed())
	{
		Vec2 cursorPos = targetWindow->GetClientCursorPos();

		CursorEvent cursorEvent(cursorPos);

		// Search hovered control to fire event on them
		cursorPos += Vec2(0.5f, 0.5f); // Important, make cursorPos pointing to the center of the pixel ! Making children gui - s non overlappable at edges
		Gui* newHoveredControl = nullptr;
		TraverseGuiControls([&](Gui& control)
		{
			if (!control.IsLayer() && control.IsHoverable() && control.GetVisiblePaddingRect().IsPointInside(cursorPos))
				newHoveredControl = &control;
		});

		if (newHoveredControl != hoveredGui)
		{
			// Cursor Leave
			if (hoveredGui)
			{
				hoveredGui->TraverseTowardParents([&](Gui& control)
				{
					control.OnCursorLeave(control, cursorEvent);
				});
			}

			// Cursor Enter
			if (newHoveredControl)
			{
				newHoveredControl->TraverseTowardParents([&](Gui& control)
				{
					if (control.GetVisiblePaddingRect().IsPointInside(cursorPos))
						control.OnCursorEnter(control, cursorEvent);
				});
			}
		}
		else
		{
			// Cursor Hover
			if (hoveredGui)
			{
				hoveredGui->TraverseTowardParents([&](Gui& control)
				{
					if (control.GetVisiblePaddingRect().IsPointInside(cursorPos) && control.IsHoverable())
					{
						control.OnCursorHover(control, cursorEvent);
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
			hoveredGui->OnOperSysDragHover(*hoveredGui, lastDropEvent);
	}
}

void GuiEngine::Render()
{
	SelectObject(memHDC, memBitmap);

	std::function<void(Gui& control)> traverseControls;
	traverseControls = [&](Gui& control)
	{
		PaintEvent paintEvent;
		paintEvent.graphics = gdiGraphics;
		control.OnPaint(control, paintEvent);
	
		for (Gui* child : control.GetChildren())
			traverseControls(*child);
	};
	
	for (Layer* layer : GetLayers())
		traverseControls(*layer);
	
	// Present
	BitBlt(hdc, 0, 0, targetWindow->GetClientSize().x, targetWindow->GetClientSize().y, memHDC, 0, 0, SRCCOPY);
}

void GuiEngine::TraverseGuiControls(const std::function<void(Gui&)>& fn)
{
	std::function<void(Gui& control, const std::function<void(Gui& control)>& fn)> traverseControls;
	traverseControls = [&](Gui& control, const std::function<void(Gui& control)>& fn)
	{
		fn(control);

		for (Gui* child : control.GetChildren())
			traverseControls(*child, fn);
	};

	auto allLayers = layers;
	allLayers.push_back(postProcessLayer);

	for (Layer* layer : allLayers)
	{
		traverseControls(*layer, [&](Gui& control)
		{
			fn(control);
		});
	}
}

void GuiEngine::SetCursorVisual(eCursorVisual cursorVisual)
{
	this->cursorVisual = cursorVisual;
	System::SetCursorVisual(cursorVisual, targetWindow->GetNativeHandle());
}

std::vector<Layer*> GuiEngine::GetLayers()
{
	std::vector<Layer*> result = layers;
	result.push_back(postProcessLayer);

	return result;
}