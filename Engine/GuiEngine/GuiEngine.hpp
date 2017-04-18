#pragma once
#include <GraphicsEngine_LL/GraphicsEngine.hpp>
#include "GuiLayer.hpp"
#include <vector>
#include <functional>

using namespace inl::gxeng;

namespace inl::gui {

class GuiEngine
{
public:
	GuiEngine(GraphicsEngine* graphicsEngine, Window* targetWindow);
	~GuiEngine();

	GuiLayer* CreateLayer();
	GuiLayer* AddLayer();

	void Update(float deltaTime);
	void Render();

	void TraverseGuiControls(const std::function<void(Gui*)>& fn);

	inline int GetWindowCursorPosX() { return targetWindow->GetClientCursorPos().x(); }
	inline int GetWindowCursorPosY() { return targetWindow->GetClientCursorPos().y(); }

	inline Window* GetTargetWindow() { return targetWindow; }

public:
	Delegate<void(CursorEvent& evt)> onMouseClick;
	Delegate<void(CursorEvent& evt)> onMousePress;
	Delegate<void(CursorEvent& evt)> onMouseRelease;
	Delegate<void(CursorEvent& evt)> onMouseMove;

	// TODO TEMPORARY GDI+, REMOVE IT OR I KILL MYSELF !!!!
	Gdiplus::Graphics* graphics;
	HDC hdc;
	HDC memHDC;
	HBITMAP memBitmap;

protected:
	GraphicsEngine* graphicsEngine;
	Window* targetWindow;

	std::vector<GuiLayer*> layers; // "layers" rendered first
	GuiLayer* postProcessLayer; // postProcessLayer renders above "layers"

	Gui* hoveredControl;
	Gui* activeContextMenu;
};

inline GuiEngine::GuiEngine(GraphicsEngine* graphicsEngine, Window* targetWindow)
:graphicsEngine(graphicsEngine), targetWindow(targetWindow), hoveredControl(nullptr), activeContextMenu(nullptr), postProcessLayer(CreateLayer())
{
	// Initialize GDI+
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	// I'm sorry but with current GDI+ gui render we need to do the rendering when the window repainting itself
	//targetWindow->hekkOnPaint += [&]() {
	//	Render();
	//};

	// Propagate mousePress
	thread_local Gui* hoveredControlOnPress = nullptr;
	thread_local Vector2i mousePosWhenPress = Vector2i(-1, -1);
	targetWindow->onMousePressed += [&](WindowEvent& event)
	{
		CursorEvent eventData;
		eventData.cursorContentPos = event.clientMousePos;
		onMousePress(eventData);

		mousePosWhenPress = event.clientMousePos;

		if (hoveredControl)
		{
			hoveredControl->TraverseTowardParents([&](Gui* control)
			{
				if (control->GetPaddingRect().IsPointInside(event.clientMousePos))
				{
					control->onMousePressed(eventData);
					control->onMousePressedClonable(control, eventData);
				}
			});
		}

		if (activeContextMenu)
			activeContextMenu->Remove();
	};

	// Propagate mouseRelease
	targetWindow->onMouseReleased += [&](WindowEvent& event)
	{
		CursorEvent eventData;
		eventData.cursorContentPos = event.clientMousePos;
		onMouseRelease(eventData);

		// Mouse click
		bool bClick = mousePosWhenPress == event.clientMousePos;

		if (bClick)
			onMouseClick(eventData);

		if (activeContextMenu)
			activeContextMenu->Remove();

		if (hoveredControl)
		{
			// Control Mouse release
			hoveredControl->TraverseTowardParents([&](Gui* control)
			{
				if (control->GetPaddingRect().IsPointInside(event.clientMousePos))
				{
					control->onMouseReleased(eventData);
					control->onMouseReleasedClonable(control, eventData);
				}
			});

			// Control Mouse click
			if (bClick)
			{
				onMouseClick(eventData);

				hoveredControl->TraverseTowardParents([&](Gui* control)
				{
					if (control->GetPaddingRect().IsPointInside(event.clientMousePos))
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
										postProcessLayer->Add(activeContextMenu);

										RectF rect = activeContextMenu->GetRect();
										rect.left = event.clientMousePos.x();
										rect.top = event.clientMousePos.y();
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
		eventData.cursorContentPos = event.clientMousePos;
		eventData.mouseDelta = event.mouseDelta;

		onMouseMove(eventData);

		if (hoveredControl)
		{
			hoveredControl->TraverseTowardParents([&](Gui* control)
			{
				if (control->GetPaddingRect().IsPointInside(event.clientMousePos))
				{
					control->onMouseMoved(eventData);
					control->onMouseMovedClonable(control, eventData);
				}
			});
		}
	};

	targetWindow->onClientSizeChanged += [this](Vector2u& size)
	{
		Vector2f newSize = Vector2f(size.x(), size.y());

		for (auto& layer : layers)
			layer->SetSize(newSize);

		postProcessLayer->SetSize(newSize);
	};
}

inline GuiEngine::~GuiEngine()
{
	for (auto& layer : layers)
		delete layer;

	layers.clear();
}

inline GuiLayer* GuiEngine::AddLayer()
{
	GuiLayer* layer = CreateLayer();
	layers.push_back(layer);
	return layer;
}

inline GuiLayer* GuiEngine::CreateLayer()
{
	return new GuiLayer(this);
}

inline void GuiEngine::Update(float deltaTime)
{
	if (!targetWindow->IsFocused())
		return;

	// Let's hint the window to repaint itself
	InvalidateRect((HWND)targetWindow->GetHandle(), NULL, true);

	TraverseGuiControls([=](Gui* control)
	{
		control->onUpdate(deltaTime);
		control->onUpdateClonable(control, deltaTime);
	});

	Vector2i cursorPos = targetWindow->GetClientCursorPos();

	CursorEvent eventData(cursorPos);

	// Search hovered control to fire event on them
	Gui* newHoveredControl = nullptr;
	TraverseGuiControls([&](Gui* control)
	{
		if (!control->IsLayer() && control->GetPaddingRect().IsPointInside(cursorPos))
			newHoveredControl = control;
	});

	if (newHoveredControl != hoveredControl)
	{
		// Cursor Leave
		if (hoveredControl)
		{
			hoveredControl->TraverseTowardParents([&](Gui* control)
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
				if (control != hoveredControl && control->GetPaddingRect().IsPointInside(cursorPos))
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
		if (hoveredControl)
		{
			hoveredControl->TraverseTowardParents([&](Gui* control)
			{
				if (control->GetPaddingRect().IsPointInside(cursorPos))
				{
					control->onMouseHovered(eventData);
					control->onMouseHoveredClonable(control, eventData);
				}
			});
		}
	}
	hoveredControl = newHoveredControl;
}

inline void GuiEngine::Render()
{
	Gdiplus::Graphics* originalGraphics = Gdiplus::Graphics::FromHWND((HWND)targetWindow->GetHandle());
	hdc = originalGraphics->GetHDC();

	RECT Content_Rect;
	GetClientRect((HWND)targetWindow->GetHandle(), &Content_Rect);
	int win_width = Content_Rect.right - Content_Rect.left;
	int win_height = Content_Rect.bottom - Content_Rect.top;

	memHDC = CreateCompatibleDC(hdc);
	memBitmap = CreateCompatibleBitmap(hdc, win_width, win_height);
	SelectObject(memHDC, memBitmap);

	graphics = Gdiplus::Graphics::FromHDC(memHDC);
	graphics->SetSmoothingMode(Gdiplus::SmoothingModeDefault);

	std::function<void(Gui* control, RectF& clipRect)> traverseControls;
	traverseControls = [&](Gui* control, RectF& clipRect)
	{
		control->OnPaint(graphics, clipRect);

		// Control the clipping rect of the children controls
		RectF rect;
		if (control->IsChildrenClipEnabled())
			rect = control->GetContentRect();
		else
			rect = RectF(-9999999, -9999999, 9999999, 9999999);

		RectF newClipRect = RectF::Intersect(clipRect, rect);

		for (Gui* child : control->GetChildren())
			traverseControls(child, newClipRect);
	};

	auto allLayers = layers;
	allLayers.push_back(postProcessLayer);

	for (GuiLayer* layer : allLayers)
	{
		RectF clipRect(-9999999, -9999999, 99999999, 99999999);
		traverseControls(layer, clipRect);

		//for (Gui* rootControl : layer->GetChildren())
		//{
		//	
		//}
	}

	// Present
	BitBlt(hdc, 0, 0, targetWindow->GetClientWidth(), targetWindow->GetClientHeight(), memHDC, 0, 0, SRCCOPY);

	// Clean up
	DeleteObject(memBitmap);
	DeleteDC(memHDC);
	DeleteDC(hdc);
}

inline void GuiEngine::TraverseGuiControls(const std::function<void(Gui*)>& fn)
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

		//for (Gui* rootControl : layer->GetChildren())
		//{
		//	traverseControls(rootControl, [&](Gui* control)
		//	{
		//		fn(control);
		//	});
		//}
	}
}

} //namespace inl::gui