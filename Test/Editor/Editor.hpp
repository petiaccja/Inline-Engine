#pragma once
#include <Core\Core.hpp>
#include <GraphicsEngine_LL\GraphicsEngine.hpp>
#include <GraphicsEngine_LL\Image.hpp>
#include <GuiEngine\GuiEngine.hpp>
#include <BaseLibrary\Platform\Window.hpp>
#include <BaseLibrary\Timer.hpp>
#include "GeneralCamera.hpp"

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
	void InitGraphicsEngine();

	void Update();

	LRESULT WndProc(HWND handle, UINT msg, WPARAM wParam, LPARAM lParam);

protected:
	inl::core::Scene* scene;
	//PerspCameraActor* camActor;
	//PerspCameraActor* cam;

	Core* core;
	InputCore* input;
	

	GuiEngine* guiEngine;
	gxeng::GraphicsEngine* graphicsEngine;
	physics::bullet::PhysicsEngineBullet* physicsEngine;

	bool bWndMaximized;

	Window* wnd;
	Window* gameWnd;

	Layer* mainLayer;

	Gui* centerRenderArea;
	Gui* captionBar;
	Image* minimizeBtn;
	Image* maximizeBtn;
	Image* closeBtn;

	GeneralCamera* cam;


	// TEMP GRAPHICS THINGS
	// Pipeline resources
	std::unique_ptr<inl::gxeng::Image> m_areaImage, m_searchImage;
	std::unique_ptr<inl::gxeng::Image> m_lensFlareColorImage;
	std::unique_ptr<inl::gxeng::Image> m_colorGradingLutImage, m_lensFlareDirtImage, m_lensFlareStarImage;
	std::unique_ptr<inl::gxeng::Image> m_fontImage;
	std::unique_ptr<std::vector<char> > m_fontBinary;
};

} // namespace inl