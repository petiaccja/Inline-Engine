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

	void SetResolution(Vector2u& size);

	void TraverseGuiControls(const std::function<void(Gui*)>& fn);

	inline Vector2i GetCursorPos() { return targetWindow->GetClientCursorPos(); }
	inline int GetCursorPosX() { return GetCursorPos().x(); }
	inline int GetCursorPosY() { return GetCursorPos().y(); }

	inline Window* GetTargetWindow() { return targetWindow; }
	Gdiplus::Graphics* GetGdiGraphics() { return gdiGraphics; }
	GuiLayer* GetPostProcessLayer() { return postProcessLayer; }

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

	Gui* hoveredControl;
	Gui* activeContextMenu;

	// TODO TEMPORARY GDI+, RATHER MAKE A RENDER INTERFACE AND IMPLEMENT DX12 ETC ABOVE IT
	Gdiplus::Graphics* gdiGraphics;
	HDC hdc;
	HDC memHDC;
	HBITMAP memBitmap;
};

inline GuiEngine::GuiEngine(GraphicsEngine* graphicsEngine, Window* targetWindow)
:graphicsEngine(graphicsEngine), targetWindow(targetWindow), hoveredControl(nullptr), activeContextMenu(nullptr), postProcessLayer(CreateLayer())
{
	gdiGraphics = nullptr;
	hdc = nullptr;
	memHDC = nullptr;

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
	thread_local Gui* hoveredControlOnPress = nullptr;
	thread_local Vector2i mousePosWhenPress = Vector2i(-1, -1);
	targetWindow->onMousePressed += [&](WindowEvent& event)
	{
		CursorEvent eventData;
		eventData.cursorContentPos = event.clientMousePos;
		onMousePressed(eventData);

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
		onMouseReleased(eventData);

		// Mouse click
		bool bClick = mousePosWhenPress == event.clientMousePos;

		if (bClick)
			onMouseClicked(eventData);

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
				onMouseClicked(eventData);

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

		onMouseMoved(eventData);

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
	for (auto& layer : layers)
		delete layer;

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
	SelectObject(memHDC, memBitmap);

	std::function<void(Gui* control, RectF& clipRect)> traverseControls;
	traverseControls = [&](Gui* control, RectF& clipRect)
	{
		control->OnPaint(gdiGraphics, clipRect);

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
	}

	// Present
	BitBlt(hdc, 0, 0, targetWindow->GetClientWidth(), targetWindow->GetClientHeight(), memHDC, 0, 0, SRCCOPY);

	// Clean up
	
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