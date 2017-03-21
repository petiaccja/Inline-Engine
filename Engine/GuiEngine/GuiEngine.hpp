#pragma once
#include "GuiCanvas.hpp"
#include <GraphicsEngine\IGraphicsEngine.hpp>
#include <vector>
#include <functional>

using namespace inl::gxeng;

class GuiEngine
{
public:
	GuiEngine(IGraphicsEngine& graphicsEngine, Window& targetWindow);
	~GuiEngine();

	void Update(float deltaTime);
	void Render();

	GuiCanvas* AddCanvas();

	void TraverseGuiControls(const std::function<void(GuiControl*)>& fn);

protected:
	IGraphicsEngine& graphicsEngine;
	Window& targetWindow;

	std::vector<GuiCanvas*> canvases;

	GuiControl* hoveredControl;
};

inline GuiEngine::GuiEngine(IGraphicsEngine& graphicsEngine, Window& targetWindow)
:graphicsEngine(graphicsEngine), targetWindow(targetWindow), hoveredControl(nullptr)
{
	// Initialize GDI+.
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	// I'm sorry but with current GDI+ gui render we need to do the rendering when the window repainting itself
	targetWindow.RegOnPaint([&]() {
		Render();
	});

	// Propagate mousePress
	thread_local GuiControl* hoveredControlOnPress = nullptr;
	targetWindow.OnMousePress += [&](WindowEvent& event)
	{
		hoveredControlOnPress = hoveredControl;

		if (hoveredControl)
		{
			hoveredControl->TraverseTowardParents([=](GuiControl* control)
			{
				control->OnCursorPress(CursorEvent(event.mousePos));
			});
		}
	};

	targetWindow.OnMouseRelease += [&](WindowEvent& event)
	{
		if (hoveredControl)
		{
			hoveredControl->TraverseTowardParents([=](GuiControl* control)
			{
				control->OnCursorRelease(CursorEvent(event.mousePos));
			});

			if (hoveredControl == hoveredControlOnPress)
			{
				hoveredControl->TraverseTowardParents([=](GuiControl* control)
				{
					control->OnCursorClick(CursorEvent(event.mousePos));
				});
			}
		}
	};
}

inline GuiEngine::~GuiEngine()
{
	for(auto& canvas : canvases)
		delete canvas;

	canvases.clear();
}

inline void GuiEngine::Update(float deltaTime)
{
	// Render
	InvalidateRect((HWND)targetWindow.GetHandle(), NULL, true);
	

	if (!targetWindow.IsFocused())
		return;

	// Search hovered control to fire event on them
	GuiControl* newHoveredControl = nullptr;
	TraverseGuiControls([&](GuiControl* guiControl)
	{
		if (guiControl->GetRect().IsPointInside(targetWindow.GetClientCursorPos()))
		{
			newHoveredControl = guiControl;
		}
	});

	

	if (newHoveredControl != hoveredControl)
	{
		if (newHoveredControl)
		{
			newHoveredControl->TraverseTowardParents([&](GuiControl* control)
			{
				control->OnCursorEnter(CursorEvent(targetWindow.GetClientCursorPos()));
			});
		}
		
		if (hoveredControl)
		{
			hoveredControl->TraverseTowardParents([&](GuiControl* control)
			{
				control->OnCursorLeave(CursorEvent(targetWindow.GetClientCursorPos()));
			});
		}
	}
	else
	{
		if (hoveredControl)
		{
			hoveredControl->TraverseTowardParents([&](GuiControl* control)
			{
				control->OnCursorStay(CursorEvent(targetWindow.GetClientCursorPos()));
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
	graphics->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

	// Draw
	TraverseGuiControls([&](GuiControl* guiControl)
	{
		guiControl->OnPaint(Memhdc, graphics);
	});

	// After draw
	BitBlt(hdc, 0, 0, win_width, win_height, Memhdc, 0, 0, SRCCOPY);
	DeleteObject(Membitmap);
	DeleteDC(Memhdc);
	DeleteDC(hdc);
	InvalidateRect((HWND)targetWindow.GetHandle(), NULL, true);
}


inline GuiCanvas* GuiEngine::AddCanvas()
{
	GuiCanvas* canvas = new GuiCanvas();
	canvases.push_back(canvas);
	return canvas;
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

	for (GuiCanvas* canvas : canvases)
	{
		for (GuiLayer* layer : canvas->GetLayers())
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
}