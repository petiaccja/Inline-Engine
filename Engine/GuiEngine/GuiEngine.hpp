#pragma once
#include "GuiCanvas.hpp"
#include <GraphicsEngine\IGraphicsEngine.hpp>
#include <vector>
#include <functional>

using namespace inl::gxeng;

class GuiEngine
{
public:
	GuiEngine(IGraphicsEngine* graphicsEngine);
	~GuiEngine();

	GuiCanvas* AddCanvas();

	void Update(float deltaTime);
	void Render();

protected:
	IGraphicsEngine* graphicsEngine;

	std::vector<GuiCanvas*> canvases;
};

inline GuiEngine::GuiEngine(IGraphicsEngine* graphicsEngine)
:graphicsEngine(graphicsEngine)
{
	GWindow->RegOnPaint([&]() {
		Render();
	});
}

inline GuiEngine::~GuiEngine()
{
	for(auto& canvas : canvases)
		delete canvas;

	canvases.clear();
}

inline GuiCanvas* GuiEngine::AddCanvas()
{
	GuiCanvas* canvas = new GuiCanvas();
	canvases.push_back(canvas);
	return canvas;
}

inline void GuiEngine::Update(float deltaTime)
{
	InvalidateRect((HWND)GWindow->GetHandle(), NULL, true);
}

inline void GuiEngine::Render()
{
	Gdiplus::Graphics* originalGraphics = Gdiplus::Graphics::FromHWND((HWND)GWindow->GetHandle());
	HDC hdc = originalGraphics->GetHDC();

	RECT Client_Rect;
	GetClientRect((HWND)GWindow->GetHandle(), &Client_Rect);
	int win_width = Client_Rect.right - Client_Rect.left;
	int win_height = Client_Rect.bottom + Client_Rect.left;
	HDC Memhdc;
	
	HBITMAP Membitmap;
	Memhdc = CreateCompatibleDC(hdc);
	Membitmap = CreateCompatibleBitmap(hdc, win_width, win_height);
	SelectObject(Memhdc, Membitmap);

	Gdiplus::Graphics* graphics = Gdiplus::Graphics::FromHDC(Memhdc);
	graphics->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

	// Draw
	std::function<void(GuiControl* control, const std::function<void(GuiControl* control)>& fn)> traverseControls;
	traverseControls = [&](GuiControl* control, const std::function<void(GuiControl* control)>& fn)
	{
		fn(control);

		for (GuiControl* child : control->GetChildren())
			traverseControls(child, fn);
	};

	for(GuiCanvas* canvas : canvases)
	{
		for (GuiLayer* layer : canvas->GetLayers())
		{
			for (GuiControl* control : layer->GetControls())
			{
				traverseControls(control, [&](GuiControl* currControl)
				{
					currControl->OnPaint(Memhdc, graphics);
				});
			}
		}
	}


	// After draw
	BitBlt(hdc, 0, 0, win_width, win_height, Memhdc, 0, 0, SRCCOPY);
	DeleteObject(Membitmap);
	DeleteDC(Memhdc);
	DeleteDC(hdc);
	//EndPaint(GWindow->GetHandle(), &ps);
}