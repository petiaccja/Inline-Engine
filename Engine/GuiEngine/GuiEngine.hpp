#pragma once
#include <GraphicsEngine_LL/GraphicsEngine.hpp>

#include "GuiLayer.hpp"
#include "GuiButton.hpp"
#include "GuiText.hpp"
#include "GuiCollapsable.hpp"
#include "GuiList.hpp"
#include "GuiSlider.hpp"
#include "GuiSplitter.hpp"

#include <vector>
#include <functional>

using namespace inl::gxeng;

namespace inl::gui {

class GuiEngine
{
	friend class Gui;
public:
	GuiEngine(GraphicsEngine* graphicsEngine, Window* targetWindow);
	~GuiEngine();

	GuiLayer* CreateLayer();
	GuiLayer* AddLayer();

	void Update(float deltaTime);
	void Render();

	void SetCursorVisual(eCursorVisual cursorVisual);

	void FreezeHover() { bHoverFreezed = true; }
	void DefreezeHover() { bHoverFreezed = false; }

	void SetResolution(Vector2u& size);

	void TraverseGuiControls(const std::function<void(Gui*)>& fn);

	inline Vector2f GetCursorPos() { return targetWindow->GetClientCursorPos(); }
	inline float GetCursorPosX() { return GetCursorPos().x(); }
	inline float GetCursorPosY() { return GetCursorPos().y(); }

	inline Window* GetTargetWindow() { return targetWindow; }
	Gdiplus::Graphics* GetGdiGraphics() { return gdiGraphics; }
	GuiLayer* GetPostProcessLayer() { return postProcessLayer; }

	bool IsHoverFreezed() { return bHoverFreezed; }
	bool IsUsingCustomCursor() { return cursorVisual != eCursorVisual::ARROW; }
	Gui* GetHoveredGui() { return hoveredGui; }

	std::vector<GuiLayer*> GetLayers();

protected:
	void Register(Gui* g) { guis.push_back(g); }

public:
	Delegate<void(CursorEvent& evt)> onMouseClicked;
	Delegate<void(CursorEvent& evt)> onMousePressed;
	Delegate<void(CursorEvent& evt)> onMouseReleased;
	Delegate<void(CursorEvent& evt)> onMouseMoved;

protected:
	GraphicsEngine* graphicsEngine;
	Window* targetWindow;

	std::vector<GuiLayer*> layers; // "layers" rendered first
	GuiLayer* postProcessLayer; // postProcessLayer renders above "layers"
	std::vector<Gui*> guis;

	Gui* hoveredGui;
	Gui* activeContextMenu;

	bool bHoverFreezed;

	eCursorVisual cursorVisual;
	// TODO TEMPORARY GDI+, RATHER MAKE A RENDER INTERFACE AND IMPLEMENT DX12 ETC ABOVE IT
	Gdiplus::Graphics* gdiGraphics;
	HDC hdc;
	HDC memHDC;
	HBITMAP memBitmap;
};

inline GuiEngine::GuiEngine(GraphicsEngine* graphicsEngine, Window* targetWindow)
{
	// TEMPORARY
	gdiGraphics = nullptr;
	hdc = nullptr;
	memHDC = nullptr;

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
	targetWindow->onClientSizeChanged += [this](Vector2u size)
	{
		SetResolution(size);
	};

	// Propagate mousePress
	thread_local Gui* hoveredGuiOnPress = nullptr;
	thread_local Vector2f mousePosWhenPress = Vector2f(-1, -1);
	targetWindow->onMousePressed += [&, targetWindow](WindowEvent& event)
	{
		CursorEvent eventData;
		eventData.cursorPos = event.clientMousePos;
		onMousePressed(eventData);

		mousePosWhenPress = event.clientMousePos;

		Vector2f pixelCenterCursorPos = event.clientMousePos + Vector2f(0.5, 0.5); // Important, make cursorPos pointing to the center of the pixel ! Making children gui - s non overlappable at edges

		if (hoveredGui)
		{
			hoveredGui->TraverseTowardParents([&](Gui* control)
			{
				if (control->GetVisibleContentRect().IsPointInside(pixelCenterCursorPos))
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
		eventData.cursorPos = event.clientMousePos;
		onMouseReleased(eventData);

		// Mouse click
		bool bClick = mousePosWhenPress == event.clientMousePos;

		if (bClick)
			onMouseClicked(eventData);

		if (activeContextMenu)
			activeContextMenu->Remove();

		Vector2f pixelCenterCursorPos = event.clientMousePos + Vector2f(0.5, 0.5); // Important, make cursorPos pointing to the center of the pixel ! Making children gui - s non overlappable at edges

		if (hoveredGui)
		{
			// Control Mouse release
			hoveredGui->TraverseTowardParents([&](Gui* control)
			{
				if (control->GetVisibleContentRect().IsPointInside(pixelCenterCursorPos))
				{
					control->onMouseReleased(eventData);
					control->onMouseReleasedClonable(control, eventData);
				}
			});

			// Control Mouse click
			if (bClick)
			{
				onMouseClicked(eventData);

				hoveredGui->TraverseTowardParents([&](Gui* control)
				{
					if (control->GetVisibleContentRect().IsPointInside(pixelCenterCursorPos))
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
		eventData.cursorPos = event.clientMousePos;
		eventData.mouseDelta = event.mouseDelta;

		onMouseMoved(eventData);

		Vector2f pixelCenterCursorPos = event.clientMousePos + Vector2f(0.5, 0.5); // Important, make cursorPos pointing to the center of the pixel ! Making children gui - s non overlappable at edges
		if (hoveredGui)
		{
			hoveredGui->TraverseTowardParents([&](Gui* control)
			{
				if (control->GetVisibleContentRect().IsPointInside(pixelCenterCursorPos))
				{
					control->onMouseMoved(eventData);
					control->onMouseMovedClonable(control, eventData);
				}
			});
		}
	};

	targetWindow->onClientSizeChanged += [this](Vector2u size)
	{
		Vector2f newSize = Vector2f(size.x(), size.y());

		for (auto& layer : layers)
			layer->SetSize(newSize);

		postProcessLayer->SetSize(newSize);
	};
}

inline GuiEngine::~GuiEngine()
{
	for (Gui* gui : guis)
		delete gui;

	layers.clear();

	DeleteObject(memBitmap);
	DeleteDC(hdc);
	DeleteDC(memHDC);
}

inline void GuiEngine::SetResolution(Vector2u& size)
{
	DeleteObject(memBitmap);
	DeleteDC(hdc);
	DeleteDC(memHDC);

	Gdiplus::Graphics* graphics = Gdiplus::Graphics::FromHWND((HWND)targetWindow->GetHandle());
	graphics->SetSmoothingMode(Gdiplus::SmoothingModeDefault);

	hdc = graphics->GetHDC();

	memHDC = CreateCompatibleDC(hdc);
	memBitmap = CreateCompatibleBitmap(hdc, size.x(), size.y());

	SelectObject(memHDC, memBitmap);

	gdiGraphics = Gdiplus::Graphics::FromHDC(memHDC);
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

	// Calculate clipping rect for all gui controls
	std::function<void(Gui* control, RectF& clipRect)> traverseControls;
	traverseControls = [&](Gui* control, RectF& clipRect)
	{
		control->SetVisibleRect(clipRect);

		// Control the clipping rect of the children controls
		RectF rect;
		if (control->IsChildrenClipEnabled())
			rect = control->GetContentRect();
		else
			rect = RectF(-FLT_MAX * 0.5, -FLT_MAX * 0.5, FLT_MAX, FLT_MAX);

		RectF newClipRect = RectF::Intersect(clipRect, rect);

		for (Gui* child : control->GetChildren())
			traverseControls(child, newClipRect);
	};

	for (GuiLayer* layer : GetLayers())
	{
		RectF clipRect(-FLT_MAX * 0.5, -FLT_MAX * 0.5, FLT_MAX, FLT_MAX);
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
		Vector2f cursorPos = targetWindow->GetClientCursorPos();

		CursorEvent eventData(cursorPos);

		// Search hovered control to fire event on them
		cursorPos += Vector2f(0.5f, 0.5f); // Important, make cursorPos pointing to the center of the pixel ! Making children gui - s non overlappable at edges
		Gui* newHoveredControl = nullptr;
		TraverseGuiControls([&](Gui* control)
		{
			if (!control->IsLayer() && control->IsHoverable() && control->GetVisibleContentRect().IsPointInside(cursorPos))
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
					if (control->GetVisibleContentRect().IsPointInside(cursorPos))
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
					if (control->GetVisibleContentRect().IsPointInside(cursorPos))
					{
						control->onMouseHovering(eventData);
						control->onMouseHoveringClonable(control, eventData);
					}
				});
			}
		}
		hoveredGui = newHoveredControl;
	}
}

inline void GuiEngine::Render()
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
	}
}

inline void GuiEngine::SetCursorVisual(eCursorVisual cursorVisual)
{
	this->cursorVisual = cursorVisual;
	Sys::SetCursorVisual(cursorVisual, targetWindow->GetHandle());
}

inline std::vector<GuiLayer*> GuiEngine::GetLayers()
{
	std::vector<GuiLayer*> result = layers;
	result.push_back(postProcessLayer);

	return result;
}

} //namespace inl::gui