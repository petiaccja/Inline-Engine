#pragma once
// The engine editor woaah

#include <Core\Core.hpp>
#include <GraphicsEngine_LL\GraphicsEngine.hpp>
#include <GuiEngine\GuiEngine.hpp>
#include <BaseLibrary\Platform\Window.hpp>
#include <BaseLibrary\Timer.hpp>

// Win32 specific window headers
#include <dwmapi.h>
#include <Windowsx.h>

using namespace std::placeholders;
using namespace exc;
using namespace inl;
using namespace inl::gui;

class Editor
{
public:
	Editor();
	~Editor();

	void InitGui();

	void Run();

	LRESULT WndProc(HWND handle, UINT msg, WPARAM wParam, LPARAM lParam);

protected:
	// TODO TEMPORARY
	class QCWorld* world;

	Core* core;
	GuiEngine* guiE;
	gxeng::GraphicsEngine* graphicsE;

	bool bWndMaximized;

	Window* wnd;
	Window* gameWnd;

	GuiLayer* mainLayer;

	Gui* captionBar;
	GuiImage* minimizeBtn;
	GuiImage* maximizeBtn;
	GuiImage* closeBtn;
};