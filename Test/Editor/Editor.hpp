#pragma once
#include <Core\Core.hpp>
#include <GraphicsEngine_LL\GraphicsEngine.hpp>
#include <GuiEngine\GuiEngine.hpp>
#include <BaseLibrary\Platform\Window.hpp>
#include <BaseLibrary\Timer.hpp>

#include <dwmapi.h>
#include <Windowsx.h>

namespace inl {

using namespace std::placeholders;
using namespace inl::core;
using namespace inl::gui;

class Editor
{
public:
	Editor();
	~Editor();

	void InitGui();
	void InitScene();

	void StartMainLoop();

	LRESULT WndProc(HWND handle, UINT msg, WPARAM wParam, LPARAM lParam);

protected:
	inl::core::Scene* scene;
	PerspCameraActor* cam;

	Core* core;
	InputCore* inputCore;
	

	GuiEngine* guiEngine;
	gxeng::GraphicsEngine* graphicsEngine;
	physics::bullet::PhysicsEngineBullet* physicsEngine;

	bool bWndMaximized;

	Window* wnd;
	Window* gameWnd;

	GuiLayer* mainLayer;

	Gui* captionBar;
	GuiImage* minimizeBtn;
	GuiImage* maximizeBtn;
	GuiImage* closeBtn;
};

} // namespace inl