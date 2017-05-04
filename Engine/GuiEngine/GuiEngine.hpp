#pragma once
#include <GraphicsEngine_LL/GraphicsEngine.hpp>

#include "GuiLayer.hpp"
#include "GuiButton.hpp"
#include "GuiText.hpp"
#include "GuiCollapsable.hpp"
#include "GuiList.hpp"
#include "GuiSlider.hpp"
#include "GuiSplitter.hpp"
#include "GuiMenu.h"

#include <vector>
#include <functional>

using namespace inl;

namespace inl::gui {

class GuiEngine
{
	friend class Gui;
public:
	GuiEngine(gxeng::GraphicsEngine* graphicsEngine, Window* targetWindow);
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
	gxeng::GraphicsEngine* graphicsEngine;
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

	// Global variables for lambdas
	Gui* hoveredGuiOnPress = nullptr;
	Vector2f mousePosWhenPress = Vector2f(-1, -1);
};

} //namespace inl::gui