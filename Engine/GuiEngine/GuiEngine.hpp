#pragma once
#include <GraphicsEngine\IGraphicsEngine.hpp>
#include "GuiLayer.hpp"
#include <vector>
#include <functional>

using namespace inl::gxeng;

class GuiEngine
{
public:
	GuiEngine(IGraphicsEngine& graphicsEngine, Window& targetWindow);
	~GuiEngine();

	GuiLayer* AddLayer();

	void Update(float deltaTime);
	void Render();

	void SetActiveSlider(GuiSlider* s) { activeSlider = s; }

	void TraverseGuiControls(const std::function<void(GuiControl*)>& fn);

	int GetWindowCursorPosX() { return targetWindow.GetClientCursorPos().x; }
	int GetWindowCursorPosY() { return targetWindow.GetClientCursorPos().y; }
	GuiSlider* GetActiveSlider() { return activeSlider; }

protected:
	IGraphicsEngine& graphicsEngine;
	Window& targetWindow;

	std::vector<GuiLayer*> layers; // "layers" rendered first
	GuiLayer* postProcessLayer; // postProcessLayer renders above "layers"

	GuiControl* hoveredControl;
	GuiControl* activeContextMenu;
	GuiSlider* activeSlider;
};



inline GuiEngine::GuiEngine(IGraphicsEngine& graphicsEngine, Window& targetWindow)
:graphicsEngine(graphicsEngine), targetWindow(targetWindow), hoveredControl(nullptr), activeContextMenu(nullptr), activeSlider(nullptr), postProcessLayer(AddLayer())
{
	// Initialize GDI+
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	// I'm sorry but with current GDI+ gui render we need to do the rendering when the window repainting itself
	targetWindow.hekkOnPaint += [&]() {
		Render();
	};

	// Propagate mousePress
	thread_local GuiControl* hoveredControlOnPress = nullptr;
	targetWindow.onMousePress += [&](WindowEvent& event)
	{
		hoveredControlOnPress = hoveredControl;

		if (hoveredControl)
		{
			hoveredControl->TraverseTowardParents([=](GuiControl* control)
			{
				control->onPress(control, CursorEvent(event.mousePos));
			});
		}

		if (activeContextMenu)
			activeContextMenu->Remove();
	};

	// Propagate mouseRelease
	targetWindow.onMouseRelease += [&](WindowEvent& event)
	{
		if (activeContextMenu)
			activeContextMenu->Remove();

		if (hoveredControl)
		{
			// Mouse release
			hoveredControl->TraverseTowardParents([=](GuiControl* control)
			{
				control->onRelease(control, CursorEvent(event.mousePos));
			});

			// Mouse click (so hoveredControl was the last guiControl we've pressed on)
			if (hoveredControl == hoveredControlOnPress)
			{
				hoveredControl->TraverseTowardParents([=](GuiControl* control)
				{
					control->onClick(control, CursorEvent(event.mousePos));

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

									Rect<float> rect = activeContextMenu->GetRect();
									rect.x = event.mousePos.x;
									rect.y = event.mousePos.y;
									activeContextMenu->SetRect(rect);
								}
							}
						}
					}
				});
			}
		}
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
	GuiLayer* layer = new GuiLayer(this);
	layers.push_back(layer);
	return layer;
}

inline void GuiEngine::Update(float deltaTime)
{
	if (!targetWindow.IsFocused())
		return;

	// Let's hint the window to repaint itself
	InvalidateRect((HWND)targetWindow.GetHandle(), NULL, true);

	TraverseGuiControls([=](GuiControl* guiControl)
	{
		guiControl->onUpdate(guiControl, deltaTime);
	});

	ivec2 cursorPos = targetWindow.GetClientCursorPos();
	
	// Search hovered control to fire event on them
	GuiControl* newHoveredControl = nullptr;
	TraverseGuiControls([&](GuiControl* guiControl)
	{
		if (guiControl->GetRect().IsPointInside(cursorPos))
		{
			newHoveredControl = guiControl;
		}
	});

	if (newHoveredControl != hoveredControl)
	{
		if (newHoveredControl)
		{
			// Cursor Enter
			newHoveredControl->TraverseTowardParents([&](GuiControl* control)
			{
				control->onCursorEnter(control, CursorEvent(cursorPos));
			});
		}
		
		if (hoveredControl)
		{
			// Cursor Leave
			hoveredControl->TraverseTowardParents([&](GuiControl* control)
			{
				control->onCursorLeave(control, CursorEvent(cursorPos));
			});
		}
	}
	else
	{
		if (hoveredControl)
		{
			// Cursor Hover
			hoveredControl->TraverseTowardParents([&](GuiControl* control)
			{
				control->onCursorHover(control, CursorEvent(cursorPos));
			});
		}
	}
	hoveredControl = newHoveredControl;
}

inline void GuiEngine::Render()
{
	Gdiplus::Graphics* originalGraphics = Gdiplus::Graphics::FromHWND((HWND)targetWindow.GetHandle());
	HDC hdc = originalGraphics->GetHDC();

	RECT Client_Rect;
	GetClientRect((HWND)targetWindow.GetHandle(), &Client_Rect);
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
	TraverseGuiControls([&](GuiControl* control)
	{
		control->onPaint(control, Memhdc, graphics);
	});

	// After draw
	BitBlt(hdc, 0, 0, win_width, win_height, Memhdc, 0, 0, SRCCOPY);
	DeleteObject(Membitmap);
	DeleteDC(Memhdc);
	DeleteDC(hdc);
	InvalidateRect((HWND)targetWindow.GetHandle(), NULL, true);
}

inline void GuiEngine::TraverseGuiControls(const std::function<void(GuiControl*)>& fn)
{
	std::function<void(GuiControl* control, const std::function<void(GuiControl* control)>& fn)> traverseControls;
	traverseControls = [&](GuiControl* control, const std::function<void(GuiControl* control)>& fn)
	{
		fn(control);

		for (GuiControl* child : control->GetChildren())
			traverseControls(child, fn);
	};

	auto allLayers = layers;
	allLayers.push_back(postProcessLayer);

	for (GuiLayer* layer : allLayers)
	{
		for (GuiControl* rootControl : layer->GetControls())
		{
			traverseControls(rootControl, [&](GuiControl* control)
			{
				fn(control);
			});
		}
	}
}