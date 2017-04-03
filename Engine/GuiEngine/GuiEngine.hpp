#pragma once
#include <GraphicsEngine\IGraphicsEngine.hpp>
#include "GuiLayer.hpp"
#include <vector>
#include <functional>

using namespace inl::gxeng;

class GuiEngine
{
public:
	GuiEngine(IGraphicsEngine* graphicsEngine, Window* targetWindow);
	~GuiEngine();

	GuiLayer* CreateLayer();
	GuiLayer* AddLayer();

	void Update(float deltaTime);
	void Render();

	void TraverseGuiControls(const std::function<void(Widget*)>& fn);

	int GetWindowCursorPosX() { return targetWindow->GetClientCursorPos().x; }
	int GetWindowCursorPosY() { return targetWindow->GetClientCursorPos().y; }

public:
	Delegate<void(CursorEvent& evt)> onMouseClick;
	Delegate<void(CursorEvent& evt)> onMousePress;
	Delegate<void(CursorEvent& evt)> onMouseRelease;
	Delegate<void(CursorEvent& evt)> onMouseMove;

protected:
	IGraphicsEngine* graphicsEngine;
	Window* targetWindow;

	std::vector<GuiLayer*> layers; // "layers" rendered first
	GuiLayer* postProcessLayer; // postProcessLayer renders above "layers"

	Widget* hoveredControl;
	Widget* activeContextMenu;
};

inline GuiEngine::GuiEngine(IGraphicsEngine* graphicsEngine, Window* targetWindow)
:graphicsEngine(graphicsEngine), targetWindow(targetWindow), hoveredControl(nullptr), activeContextMenu(nullptr), postProcessLayer(CreateLayer())
{
	// Initialize GDI+
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	// I'm sorry but with current GDI+ gui render we need to do the rendering when the window repainting itself
	targetWindow->hekkOnPaint += [&]() {
		Render();
	};

	// Propagate mousePress
	thread_local Widget* hoveredControlOnPress = nullptr;
	thread_local ivec2 mousePosWhenPress = ivec2(-1, -1);
	targetWindow->onMousePress += [&](WindowEvent& event)
	{
		CursorEvent eventData;
		eventData.cursorClientPos = event.mousePos;
		onMousePress(eventData);

		//hoveredControlOnPress = hoveredControl;
		mousePosWhenPress = event.mousePos;

		if (hoveredControl)
		{
			hoveredControl->TraverseTowardParents([&](Widget* control)
			{
				if(control->IsPointInside(event.mousePos))
					control->onMousePress(control, eventData);
			});
		}

		if (activeContextMenu)
			activeContextMenu->Remove();
	};

	// Propagate mouseRelease
	targetWindow->onMouseRelease += [&](WindowEvent& event)
	{
		CursorEvent eventData;
		eventData.cursorClientPos = event.mousePos;
		onMouseRelease(eventData);

		// Mouse click
		bool bClick = mousePosWhenPress == event.mousePos;

		if (bClick)
			onMouseClick(eventData);

		if (activeContextMenu)
			activeContextMenu->Remove();

		if (hoveredControl)
		{
			// Control Mouse release
			hoveredControl->TraverseTowardParents([&](Widget* control)
			{
				if (control->IsPointInside(event.mousePos))
					control->onMouseRelease(control, eventData);
			});

			// Control Mouse click
			if (bClick)
			{
				onMouseClick(eventData);

				hoveredControl->TraverseTowardParents([&](Widget* control)
				{
					if (control->IsPointInside(event.mousePos))
					{
						control->onMouseClick(control, eventData);

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
										postProcessLayer->AddControl(activeContextMenu);

										Rect<float> rect = activeContextMenu->GetRect();
										rect.x = event.mousePos.x;
										rect.y = event.mousePos.y;
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
	targetWindow->onMouseMove += [&](WindowEvent& event)
	{
		CursorEvent eventData;
		eventData.cursorClientPos = event.mousePos;

		onMouseMove(eventData);
	};
}

inline GuiEngine::~GuiEngine()
{
	for(auto& layer : layers)
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

	TraverseGuiControls([=](Widget* Widget)
	{
		Widget->onUpdate(Widget, deltaTime);
	});

	ivec2 cursorPos = targetWindow->GetClientCursorPos();
	
	// Search hovered control to fire event on them
	Widget* newHoveredControl = nullptr;
	TraverseGuiControls([&](Widget* control)
	{
		if (control->IsPointInside(cursorPos))
			newHoveredControl = control;
	});

	if (newHoveredControl != hoveredControl)
	{
		// Cursor Leave
		if (hoveredControl)
		{
			hoveredControl->TraverseTowardParents([&](Widget* control)
			{
				control->onMouseLeave(control, CursorEvent(cursorPos));
			});
		}

		// Cursor Enter
		if (newHoveredControl)
		{
			newHoveredControl->TraverseTowardParents([&](Widget* control)
			{
				if (control->IsPointInside(cursorPos))
					control->onMouseEnter(control, CursorEvent(cursorPos));
			});
		}
	}
	else
	{
		// Cursor Hover
		if (hoveredControl)
		{
			hoveredControl->TraverseTowardParents([&](Widget* control)
			{
				if(control->IsPointInside(cursorPos))
					control->onMouseHover(control, CursorEvent(cursorPos));
			});
		}
	}
	hoveredControl = newHoveredControl;
}

inline void GuiEngine::Render()
{
	Gdiplus::Graphics* originalGraphics = Gdiplus::Graphics::FromHWND((HWND)targetWindow->GetHandle());
	HDC hdc = originalGraphics->GetHDC();

	RECT Client_Rect;
	GetClientRect((HWND)targetWindow->GetHandle(), &Client_Rect);
	int win_width = Client_Rect.right - Client_Rect.left;
	int win_height = Client_Rect.bottom - Client_Rect.top;
	HDC Memhdc;
	
	HBITMAP Membitmap;
	Memhdc = CreateCompatibleDC(hdc);
	Membitmap = CreateCompatibleBitmap(hdc, win_width, win_height);
	SelectObject(Memhdc, Membitmap);

	Gdiplus::Graphics* graphics = Gdiplus::Graphics::FromHDC(Memhdc);
	graphics->SetSmoothingMode(Gdiplus::SmoothingModeDefault);

	// Draw
	//TraverseGuiControls([&](Widget* control)
	//{
	//	control->onPaint(control, graphics);
	//});


	std::function<void(Widget* control, Rect<float>& clipRect)> traverseControls;
	traverseControls = [&](Widget* control, Rect<float>& clipRect)
	{
		control->onPaint(control, graphics, clipRect);

		Rect<float> newClipRect = clipRect.Intersect(control->GetClientRect());

		for (Widget* child : control->GetChildren())
			traverseControls(child, newClipRect);
	};

	auto allLayers = layers;
	allLayers.push_back(postProcessLayer);

	for (GuiLayer* layer : allLayers)
	{
		Rect<float> clipRect(0, 0, 9999, 9999);
		for (Widget* rootControl : layer->GetControls())
		{
			traverseControls(rootControl, clipRect);
		}
	}


	// After draw
	BitBlt(hdc, 0, 0, win_width, win_height, Memhdc, 0, 0, SRCCOPY);
	DeleteObject(Membitmap);
	DeleteDC(Memhdc);
	DeleteDC(hdc);
}

inline void GuiEngine::TraverseGuiControls(const std::function<void(Widget*)>& fn)
{
	std::function<void(Widget* control, const std::function<void(Widget* control)>& fn)> traverseControls;
	traverseControls = [&](Widget* control, const std::function<void(Widget* control)>& fn)
	{
		fn(control);

		for (Widget* child : control->GetChildren())
			traverseControls(child, fn);
	};

	auto allLayers = layers;
	allLayers.push_back(postProcessLayer);

	for (GuiLayer* layer : allLayers)
	{
		for (Widget* rootControl : layer->GetControls())
		{
			traverseControls(rootControl, [&](Widget* control)
			{
				fn(control);
			});
		}
	}
}