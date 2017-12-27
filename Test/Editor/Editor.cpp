#include "Editor.hpp"
#include <Core\TimeCore.hpp>
#include <GraphicsEngine_LL\Pixel.hpp>
#include <AssetLibrary\Image.hpp>
#include "AreaTex.h"
#include "SearchTex.h"

namespace inl {

Editor::Editor()
{
	bWndMaximized = false;

	core = new Core();
	input = new InputCore();

	// Create main window for Editor
	wnd = new Window("Inline Engine", Vec2u(800, 600), false, true, false);

	wnd->OnPaint += [this]()
	{
		if (guiEngine)
			guiEngine->Render();
	};

	// Create secondary window for GAME inside Editor
	//gameWnd = new Window("Inline Engine", Vec2u(100, 100), true, true);

	//HWND editorHwnd = (HWND)wnd->GetNativeHandle();
	//HWND gameHwnd = (HWND)gameWnd->GetNativeHandle();
	//SetParent(gameHwnd, editorHwnd);

	// Resize window, non client area removal made it's size wrong
	//wnd->SetRect({ 0,0 }, Sys::GetScreenSize());

	// Init Graphics Engine
	graphicsEngine = core->InitGraphicsEngine(wnd->GetClientSize().x, wnd->GetClientSize().y, (HWND)wnd->GetNativeHandle());

	InitGraphicsEngine();

	// Init Physics Engine
	physicsEngine = core->InitPhysicsEngineBullet();

	// Init Gui Engine
	guiEngine = core->InitGuiEngine(graphicsEngine, wnd);

	InitScene();

	// Init Editor Gui
	InitGui();

	// Set icon for the editor window
	wnd->SetIcon("InlineEngineLogo.ico");
}

Editor::~Editor()
{
	//delete core;
	delete wnd;
	//delete gameWnd;
}

void Editor::InitScene()
{
	// Create scene for editor
	scene = core->CreateScene();

	//camActor = scene->AddActor_PerspCamera();
	//camActor->SetNearPlaneDist(0.1);
	//camActor->SetFarPlaneDist(2000.0);

	cam = new GeneralCamera(scene, input, core->GetGraphicsEngine()->CreatePerspectiveCamera("WorldCam"), centerRenderArea);
	cam->SetNearPlaneDist(0.1);
	cam->SetFarPlaneDist(2000.0);
	scene->AddActor(cam);

	// TODO
	//cam->SetPos({ 2, -10, 9 });
	//cam->SetTarget({ 1, 1, 10 });

	cam->SetPos({ 0,0,10 });
	cam->SetTarget({ 0,1,10});
}

void Editor::InitGraphicsEngine()
{
	graphicsEngine->CreateScene("Gui");
	graphicsEngine->CreateOrthographicCamera("GuiCamera");
	graphicsEngine->SetEnvVariable("world_render_pos", inl::Any(Vec2(0.f, 0.f)));
	graphicsEngine->SetEnvVariable("world_render_rot", inl::Any(0.f));

	// Create pipeline resources
	using PixelT = gxeng::Pixel<gxeng::ePixelChannelType::INT8_NORM, 2, gxeng::ePixelClass::LINEAR>;

	{
		m_areaImage.reset(graphicsEngine->CreateImage());
		m_areaImage->SetLayout(AREATEX_WIDTH, AREATEX_HEIGHT, gxeng::ePixelChannelType::INT8_NORM, 2, gxeng::ePixelClass::LINEAR);
		m_areaImage->Update(0, 0, AREATEX_WIDTH, AREATEX_HEIGHT, 0, areaTexBytes, PixelT::Reader());

		graphicsEngine->SetEnvVariable("SMAA_areaTex", inl::Any{ m_areaImage.get() });
	}

	{
		using PixelT = gxeng::Pixel<gxeng::ePixelChannelType::INT8_NORM, 1, gxeng::ePixelClass::LINEAR>;
		m_searchImage.reset(graphicsEngine->CreateImage());
		m_searchImage->SetLayout(SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT, gxeng::ePixelChannelType::INT8_NORM, 1, gxeng::ePixelClass::LINEAR);
		m_searchImage->Update(0, 0, SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT, 0, searchTexBytes, PixelT::Reader());

		graphicsEngine->SetEnvVariable("SMAA_searchTex", inl::Any{ m_searchImage.get() });
	}

	{
		using PixelT = gxeng::Pixel<gxeng::ePixelChannelType::INT8_NORM, 4, gxeng::ePixelClass::LINEAR>;
		inl::asset::Image img("assets\\lensFlare\\lens_color.png");

		m_lensFlareColorImage.reset(graphicsEngine->CreateImage());
		m_lensFlareColorImage->SetLayout(img.GetWidth(), img.GetHeight(), gxeng::ePixelChannelType::INT8_NORM, 4, gxeng::ePixelClass::LINEAR);
		m_lensFlareColorImage->Update(0, 0, img.GetWidth(), img.GetHeight(), 0, img.GetData(), PixelT::Reader());

		graphicsEngine->SetEnvVariable("LensFlare_ColorTex", inl::Any{ m_lensFlareColorImage.get() });
	}

	{
		using PixelT = gxeng::Pixel<gxeng::ePixelChannelType::INT8_NORM, 3, gxeng::ePixelClass::LINEAR>;
		inl::asset::Image img("assets\\colorGrading\\default_lut_table.png");

		m_colorGradingLutImage.reset(graphicsEngine->CreateImage());
		m_colorGradingLutImage->SetLayout(img.GetWidth(), img.GetHeight(), gxeng::ePixelChannelType::INT8_NORM, 3, gxeng::ePixelClass::LINEAR);
		m_colorGradingLutImage->Update(0, 0, img.GetWidth(), img.GetHeight(), 0, img.GetData(), PixelT::Reader());

		//TODO
		//create cube texture

		graphicsEngine->SetEnvVariable("HDRCombine_colorGradingTex", inl::Any{ m_colorGradingLutImage.get() });
	}

	{
		using PixelT = gxeng::Pixel<gxeng::ePixelChannelType::INT8_NORM, 4, gxeng::ePixelClass::LINEAR>;
		inl::asset::Image img("assets\\lensFlare\\lens_dirt.png");

		m_lensFlareDirtImage.reset(graphicsEngine->CreateImage());
		m_lensFlareDirtImage->SetLayout(img.GetWidth(), img.GetHeight(), gxeng::ePixelChannelType::INT8_NORM, 4, gxeng::ePixelClass::LINEAR);
		m_lensFlareDirtImage->Update(0, 0, img.GetWidth(), img.GetHeight(), 0, img.GetData(), PixelT::Reader());

		graphicsEngine->SetEnvVariable("HDRCombine_lensFlareDirtTex", inl::Any{ m_lensFlareDirtImage.get() });
	}

	{
		using PixelT = gxeng::Pixel<gxeng::ePixelChannelType::INT8_NORM, 4, gxeng::ePixelClass::LINEAR>;
		inl::asset::Image img("assets\\lensFlare\\lens_star.png");

		m_lensFlareStarImage.reset(graphicsEngine->CreateImage());
		m_lensFlareStarImage->SetLayout(img.GetWidth(), img.GetHeight(), gxeng::ePixelChannelType::INT8_NORM, 4, gxeng::ePixelClass::LINEAR);
		m_lensFlareStarImage->Update(0, 0, img.GetWidth(), img.GetHeight(), 0, img.GetData(), PixelT::Reader());

		graphicsEngine->SetEnvVariable("HDRCombine_lensFlareStarTex", inl::Any{ m_lensFlareStarImage.get() });
	}

	{
		using PixelT = gxeng::Pixel<gxeng::ePixelChannelType::INT8_NORM, 4, gxeng::ePixelClass::LINEAR>;
		inl::asset::Image img("assets\\font\\courier_new_0.png");

		m_fontImage.reset(graphicsEngine->CreateImage());
		m_fontImage->SetLayout(img.GetWidth(), img.GetHeight(), gxeng::ePixelChannelType::INT8_NORM, 4, gxeng::ePixelClass::LINEAR);
		m_fontImage->Update(0, 0, img.GetWidth(), img.GetHeight(), 0, img.GetData(), PixelT::Reader());

		graphicsEngine->SetEnvVariable("TextRender_fontTex", inl::Any{ m_fontImage.get() });
	}

	{
		std::fstream f;
		f.open("assets\\font\\courier_new.fnt", std::ios::in | std::ios::binary | std::ios::ate);
		if (f.is_open())
		{
			size_t size = f.tellg();
			m_fontBinary.reset(new std::vector<char>);
			m_fontBinary->resize(size);
			f.seekg(0, std::ios::beg);
			f.read(m_fontBinary->data(), size);
			f.close();
		}

		graphicsEngine->SetEnvVariable("TextRender_fontBinary", inl::Any{ m_fontBinary.get() });
	}
}

void Editor::InitGui()
{
	// New Layer
	mainLayer = guiEngine->AddLayer();

	// Layer border
	mainLayer->SetBorder(1, ColorI(100, 100, 100, 255));

	// Main layout of the editor is a simple list
	GuiList* mainLayout = mainLayer->AddGui<GuiList>();
	mainLayout->StretchFillParent(); // Fill the layer
	mainLayout->SetOrientation(eGuiOrientation::VERTICAL);
	mainLayout->SetBgToColor(ColorI(0,0,0,255));

	// Caption bar
	captionBar = mainLayer->AddGui();
	captionBar->SetBgToColor(ColorI(45, 45, 45, 255));
	captionBar->SetRect(0, 0, 100, 26);

	// Minimize, Maximize, Close btn
	GuiList* minMaxCloseList = mainLayer->AddGui<GuiList>();
	minMaxCloseList->StretchFitToChildren();
	minimizeBtn = mainLayer->AddGui<GuiImage>();
	maximizeBtn = mainLayer->AddGui<GuiImage>();
	closeBtn = mainLayer->AddGui<GuiImage>();

	minimizeBtn->OnCursorClicked += [this](Gui& self, CursorEvent& evt) { wnd->Minize(); };
	maximizeBtn->OnCursorClicked += [this](Gui& self, CursorEvent& evt)
	{
		if (bWndMaximized)
			wnd->Restore();
		else
			wnd->Maximize();
	};
	closeBtn->OnCursorClicked += [this](Gui& self, CursorEvent& evt) { /*wnd->Close(); TODO*/ };

	minimizeBtn->SetImages(L"Resources/minimize.png", L"Resources/minimize_h.png");
	maximizeBtn->SetImages(L"Resources/maximize.png", L"Resources/maximize_h.png");
	closeBtn->SetImages(L"Resources/close.png", L"Resources/close_h.png");

	minMaxCloseList->SetOrientation(eGuiOrientation::HORIZONTAL);
	minMaxCloseList->AddItem(minimizeBtn);
	minMaxCloseList->AddItem(maximizeBtn);
	minMaxCloseList->AddItem(closeBtn);
	minMaxCloseList->AlignRight();

	// Editor caption text
	GuiText* inlineEngineText = mainLayer->AddGui<GuiText>();
	inlineEngineText->SetFontSize(14);
	inlineEngineText->SetFontStyle(Gdiplus::FontStyle::FontStyleBold);
	inlineEngineText->SetText("Inline Editor");
	inlineEngineText->AlignVerCenter();
	inlineEngineText->StretchHorFillParent();
	inlineEngineText->SetMarginLeft(7);

	captionBar->AddGui(inlineEngineText);
	captionBar->AddGui(minMaxCloseList);
	captionBar->StretchHorFillParent();

	mainLayout->AddItem(captionBar);

	// Main menu bar
	GuiMenu* menuBar = mainLayer->AddGui<GuiMenu>();
	menuBar->SetBorder(0, 0, 0, 1, ColorI(70, 70, 70, 255));
	menuBar->SetOrientation(eGuiOrientation::HORIZONTAL);
	menuBar->SetBgToColor(ColorI(25, 25, 25, 255));
	menuBar->StretchHorFillParent();
	menuBar->StretchVerFitToChildren();
	{
		GuiMenu* fileMenu = menuBar->AddItemMenu("File");
		GuiMenu* buildMenu = menuBar->AddItemMenu("Build");
		GuiMenu* toolsMenu = menuBar->AddItemMenu("Tools");
		GuiMenu* helpMenu = menuBar->AddItemMenu("Help");

		//fileMenu->GetGuiButtonText()->SetFontSize(18);
		//buildMenu->GetGuiButtonText()->SetFontSize(18);
		//toolsMenu->GetGuiButtonText()->SetFontSize(18);
		//helpMenu->GetGuiButtonText()->SetFontSize(18);

		fileMenu->AddItemButton("New Scene");
		fileMenu->AddItemButton("Open Scene");
		Gui* separator0 = fileMenu->AddItemSeparatorHor();
		separator0->SetBgToColor(ColorI(80, 80, 80, 255));
		separator0->DisableHover();
		fileMenu->AddItemButton("Save Scene");
		fileMenu->AddItemButton("Save Scene as...");
		Gui* separator1 = fileMenu->AddItemSeparatorHor();
		separator1->SetBgToColor(ColorI(80, 80, 80, 255));
		separator1->DisableHover();
		fileMenu->AddItemButton("New Project...");
		fileMenu->AddItemButton("Open Project...");
		fileMenu->AddItemButton("Save Project");

		buildMenu->AddItemButton("Windows...");
		buildMenu->AddItemButton("Linux...");
		buildMenu->AddItemButton("Mac...");
		buildMenu->AddItemButton("XBox One...");
		buildMenu->AddItemButton("PS4...");
		buildMenu->AddItemButton("Android");
		buildMenu->AddItemButton("IOS");

		GuiMenu* menu0 = toolsMenu->AddItemMenu("TESZT - 0");
		GuiMenu* menu1 = menu0->AddItemMenu("TESZT - 1");
		GuiMenu* menu2 = menu1->AddItemMenu("TESZT - 2");

		GuiMenu* menu00 = toolsMenu->AddItemMenu("TESZT - 00");
		GuiMenu* menu01 = menu00->AddItemMenu("TESZT - 01");
		GuiMenu* menu001 = menu00->AddItemMenu("TESZT - 001");
		GuiMenu* menu02 = menu01->AddItemMenu("TESZT - 02");
		GuiMenu* menu002 = menu001->AddItemMenu("TESZT - 002");
		toolsMenu->AddItemButton("** PUT TOOLS HERE **");

		helpMenu->AddItemButton("About Inline Engine");


		for (Gui* c : { fileMenu, buildMenu, toolsMenu, helpMenu, menu0, menu1, menu00, menu01, menu001, menu02, menu002 })
		{
			c->SetBorder(1, ColorI(70, 70, 70, 255));
			for (Gui* child : c->GetChildrenRecursive<GuiButton>())
			{
				child->SetBgToColor(ColorI(25, 25, 25, 255), ColorI(65, 65, 65, 255));
				child->SetPadding(2, 2, 4, 4);
			}
		}

		for (Gui* c : menuBar->GetChildrenRecursive<GuiButton>())
		{
			c->SetBgToColor(ColorI(25, 25, 25, 255), ColorI(65, 65, 65, 255));
			c->StretchFitToChildren();
			c->SetPadding(4, 4, 2, 2);
			c->AlignCenter();
		}
	}
	mainLayout->AddItem(menuBar);

	//Gui* toolBar = mainLayer->AddGui();
	//toolBar->StretchHorFillParent();
	//toolBar->SetBgToColor(ColorI(25));
	//mainLayout->AddItem(toolBar);

	GuiSplitter* split0 = mainLayer->AddGui<GuiSplitter>(); // split main
	GuiSplitter* split1 = mainLayer->AddGui<GuiSplitter>(); // split main left to (top, bottom)
	GuiSplitter* split2 = mainLayer->AddGui<GuiSplitter>(); // split main left-top to (left, right)
	split0->Stretch(eGuiStretch::FILL_PARENT_POSITIVE_DIR, eGuiStretch::FILL_PARENT_POSITIVE_DIR);
	split1->StretchFillParent();
	split2->StretchFillParent();

	split0->SetOrientation(eGuiOrientation::HORIZONTAL);
	split1->SetOrientation(eGuiOrientation::VERTICAL);
	split2->SetOrientation(eGuiOrientation::HORIZONTAL);

	//split0->SetSize(400, 400);
	split1->SetSize(750, 400);
	split2->SetSize(750, 300);

	Gui* rightArea = mainLayer->AddGui<GuiButton>();
	Gui* bottomArea = mainLayer->AddGui<GuiButton>();
	Gui* leftArea = mainLayer->AddGui<GuiButton>();
	centerRenderArea = mainLayer->AddGui<GuiButton>();
	rightArea->SetSize(150, 100);
	rightArea->SetBgToColor(ColorI(35, 35, 35, 255));
	bottomArea->SetSize(100, 100);
	bottomArea->SetBgToColor(ColorI(35, 35, 35, 255));
	leftArea->SetSize(30, 100);
	leftArea->SetBgToColor(ColorI(35, 35, 35, 255));
	centerRenderArea->SetSize(100, 100);
	centerRenderArea->SetBgToColor(ColorI(0, 0, 0, 255));
	centerRenderArea->StretchFillParent();

	centerRenderArea->OnCursorClicked += [this](Gui& self, CursorEvent& evt)
	{
		if (evt.mouseButton == eMouseButton::LEFT)
		{
			Ray3D ray = cam->ScreenPointToRay(centerRenderArea->GetCursorPosContentSpace());
			TraceResult traceResult;

			Vec3 spawnPos;
			if (scene->TraceGraphicsRay(ray, traceResult))
			{
				spawnPos = traceResult.pos;
			}
			else
			{
				spawnPos = cam->GetPos() + cam->GetFrontDir() * 200;
			}

			// Spawn something at that position
			RigidBodyActor* actor = scene->AddActor_RigidBody("D:/sphere2.fbx", 0);
			MeshActor* childActor = actor->AddActor_Mesh("D:/sphere2.fbx");

			actor->SetPos(spawnPos);
		}
		
	};

	centerRenderArea->OnTransformChanged += [this](Gui& self, TransformEvent& e)
	{
		//HWND gameHwnd = (HWND)gameWnd->GetNativeHandle();
		//HWND editorHwnd = (HWND)this->wnd->GetNativeHandle();
		//SetWindowPos(gameHwnd, NULL, self.GetContentPos().x, self.GetContentPos().y, self.GetContentSize().x, self.GetContentSize().y, 0);
		//SetFocus(editorHwnd);

		int width = e.rect.GetWidth();
		int height = e.rect.GetHeight();

		float aspectRatio = (float)width / height;

		graphicsEngine->SetScreenSize(width, height);
		graphicsEngine->SetEnvVariable("world_render_size", inl::Any(Vec2(width, height)));
		
		cam->SetAspectRatio(aspectRatio);
		cam->SetFOV(60.f / 180.f * 3.14159265);
		cam->SetViewportRect(RectF(0, width, 0, height));
	};

	rightArea->StretchFillParent();
	bottomArea->StretchFillParent();
	leftArea->StretchFillParent();

	split0->AddItem(split1);
	split0->AddItem(rightArea);

	split1->AddItem(split2);
	split1->AddItem(bottomArea);

	split2->AddItem(leftArea);
	split2->AddItem(centerRenderArea);
	mainLayout->AddItem(split0);

	for (auto& splitter : { split0, split1, split2 })
	{
		for (auto& separator : splitter->GetSeparators())
		{
			if (splitter->GetOrientation() == eGuiOrientation::HORIZONTAL)
				separator->SetBorder(1, 1, 0, 0, ColorI(0,0,0,255));
			else
				separator->SetBorder(0, 0, 1, 1, ColorI(0,0,0,255));
		}
	}

	GuiScrollable* scrollableBottom = bottomArea->AddGui<GuiScrollable>();
	scrollableBottom->StretchFillParent();


	GuiList* textureList = new GuiList(scrollableBottom->guiEngine);
	scrollableBottom->SetContent(textureList);

	textureList->SetBgToColor(ColorI(0, 0, 0, 0));
	textureList->SetOrientation(eGuiOrientation::HORIZONTAL);
	textureList->StretchFitToChildren();
	textureList->SetName(L"TEXTURE_LIST");



	Gui* contentCell = scrollableBottom->GetCell(0, 0);
	thread_local float colorDiff = 0;
	thread_local float time = 0;
	contentCell->OnOperSysDragEntered += [contentCell](Gui& self, DragDropEvent& data)
	{
		time = 0;
		// Show tooltip about what will happen if user Drops it
	};

	contentCell->OnOperSysDragLeaved += [contentCell](Gui& self, DragDropEvent& data)
	{
		// Remove the highlight
		contentCell->SetBgActiveColor(contentCell->GetBgIdleColor());
		
		// Hide tooltip
	};

	contentCell->OnOperSysDragHovering += [contentCell](Gui& self, DragDropEvent& data)
	{
		const float maxColorDifference = 40;

		time += Time.deltaTime * 5;
		int color = (int)(sin(time) * maxColorDifference);
		contentCell->SetBgActiveColor(contentCell->GetBgIdleColor() + ColorI(color, color, color, 255));
	};

	contentCell->OnOperSysDropped += [this, textureList, contentCell](Gui& self, DragDropEvent& e)
	{
		contentCell->SetBgActiveColor(contentCell->GetBgIdleColor());

		std::vector<path> filesPaths = e.filePaths;
		std::string text = e.text;

		for (int i = 0; i < filesPaths.size(); ++i)
		{
			path filePath = filesPaths[i];

			// Texture image
			GuiList* listItem = textureList->AddItem<GuiList>();
			listItem->StretchFitToChildren();
			listItem->MakeVertical();
			listItem->SetSize(70, 100);
			listItem->SetBgToColor(ColorI(0, 0, 0, 0), ColorI(20, 20, 20, 255));
			GuiImage* img0 = listItem->AddItem<GuiImage>();
			img0->AlignHorCenter();
			img0->SetMargin(4, 4, 4, 0);
			img0->SetImage(filePath.c_str(), 70, 70);
			img0->SetSize(70, 70);

			listItem->OnCursorClicked += [filePath, this](Gui& self, CursorEvent& evt)
			{
				if (filePath.extension() == L".fbx")
				{
					scene->AddActor_Mesh(filePath);
				}
			};
			
			filePath.replace_extension("");
			std::wstring nameWithoutExt = filePath.filename();

			// Texture text
			GuiText* text0 = listItem->AddGui<GuiText>();
			text0->StretchFitToChildren();
			text0->AlignHorCenter();
			text0->SetMargin(4);
			text0->SetText(nameWithoutExt);
			listItem->DisableChildrenHover();
		}

		SetFocus((HWND)wnd->GetNativeHandle());
	};
}

void Editor::Update()
{
	// Create timer, delta time -> engine
	Timer* timer = new Timer();
	timer->Start();

	wnd->SetTitle("Inline Editor");

	wnd->SetQueueMode(eInputQueueMode::IMMEDIATE);
	while (!wnd->IsClosed())
	{
		// Prepare for input processing
		input->ClearFrameData();
	
		wnd->CallEvents();

		//WindowEvent evt;
		//
		//// TODO TEMPORARY TILL GDI REMOVAL
		//Delegate<void(Vec2&)> doHekk;
		//doHekk += [this](Vec2& pos)
		//{
		//	if (centerRenderArea->IsCursorInside())
		//	{
		//		POINT p;
		//		p.x = pos.x;
		//		p.y = pos.y;
		//		ClientToScreen((HWND)gameWnd->GetHandle(), &p);
		//		ScreenToClient((HWND)wnd->GetHandle(), &p);
		//
		//		pos = Vec2(p.x, p.y);
		//	}
		//};
		//wnd->SetHekkTillGdiNotRemoved(doHekk);
		//
		//while(wnd->PopEvent(evt))
		//{
		//	
		//
		//	switch (evt.msg)
		//	{
		//		case eWindowMsg::KEY_PRESS:
		//		{
		//			if (evt.key != eKey::INVALID)
		//				input->SimulateKeyPress(evt.key);
		//		} break;
		//
		//		case eWindowMsg::KEY_RELEASE:
		//		{
		//			if (evt.key != eKey::INVALID)
		//				input->SimulateKeyRelease(evt.key);
		//		} break;
		//
		//		case eWindowMsg::MOUSE_MOVE:
		//		{
		//			input->SimulateMouseMove(Vec2i(evt.mouseDelta.x, evt.mouseDelta.y), evt.clientCursorPos);
		//		} break;
		//
		//		case eWindowMsg::MOUSE_PRESS:
		//		{
		//			input->SimulateMouseBtnPress(evt.mouseBtn);
		//		} break;
		//
		//		case eWindowMsg::MOUSE_RELEASE:
		//		{
		//			input->SimulateMouseBtnRelease(evt.mouseBtn);
		//		} break;
		//	}
		//}
	
		// Dispatch Inputs
		//input->Update();
	
		// Frame delta time
		Time.deltaTime = timer->Elapsed();
		timer->Reset();
	
		// Update editor camera
		if(centerRenderArea->IsCursorInside())
			cam->Update(Time.deltaTime);
	
		// Update game world
		//world->UpdateWorld(Time.deltaTime);
	
		// Update engine
		core->Update(Time.deltaTime);
	}

	delete timer;
}

LRESULT Editor::WndProc(HWND handle, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRet;
	bool fCallDWP = !DwmDefWindowProc(handle, msg, wParam, lParam, &lRet);

	switch (msg)
	{
	case WM_SETCURSOR:
		if (LOWORD(lParam) == HTCLIENT)
		{
			if (!guiEngine->IsUsingCustomCursor())
				SetCursor(LoadCursor(nullptr, IDC_ARROW));

			return TRUE;
		}
		break;
	case WM_ACTIVATE:
	{
		// Extend the frame into the client area.
		MARGINS margins;

		margins.cxLeftWidth = 0;		// 8
		margins.cxRightWidth = 0;		// 8
		margins.cyBottomHeight = 0;		// 20
		margins.cyTopHeight = 0;		// 27

		HRESULT hr = DwmExtendFrameIntoClientArea(handle, &margins);
		assert(hr == S_OK);

		fCallDWP = true;
		lRet = 0;

		break;
	}
	case WM_CREATE:
	{
		RECT rcClient;
		GetWindowRect(handle, &rcClient);

		// Inform the application of the frame change.
		SetWindowPos(handle,
			NULL,
			rcClient.left, rcClient.top,
			rcClient.right - rcClient.left, rcClient.top - rcClient.bottom,
			SWP_FRAMECHANGED);

		fCallDWP = true;
		lRet = 0;

		break;
	}
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		BeginPaint(handle, &ps);

		if (guiEngine)
			guiEngine->Render();

		EndPaint(handle, &ps);
		fCallDWP = true;
		lRet = 0;
		break;
	}
	case WM_SIZE:
	{
		if (maximizeBtn)
		{
			if (wParam == SIZE_MAXIMIZED)
			{
				maximizeBtn->SetImages(L"Resources/restore.png", L"Resources/restore_h.png");
				bWndMaximized = true;
			}
			else if (wParam == SIZE_RESTORED)
			{
				maximizeBtn->SetImages(L"Resources/maximize.png", L"Resources/maximize_h.png");
				
				bWndMaximized = false;
			}
		}

		break;
	}
	case WM_DESTROY:
	{
		PostQuitMessage(WM_QUIT);
		break;
	}
	case WM_NCCALCSIZE:
	{
		// Calculate new NCCALCSIZE_PARAMS based on custom NCA inset.
		NCCALCSIZE_PARAMS *pncsp = reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);

		pncsp->rgrc[0].left = pncsp->rgrc[0].left + 0;
		pncsp->rgrc[0].top = pncsp->rgrc[0].top + 0;
		pncsp->rgrc[0].right = pncsp->rgrc[0].right - 0;
		pncsp->rgrc[0].bottom = pncsp->rgrc[0].bottom - 0;

		lRet = 0;

		// No need to pass the message on to the DefWindowProc.
		fCallDWP = false;

		break;
	}
	case WM_NCHITTEST:
	{
		Vec2 cursorPos = guiEngine->GetCursorPos();

		bool bLeft = cursorPos.x < 8;
		bool bRight = cursorPos.x > mainLayer->GetWidth() - 8;
		bool bTop = cursorPos.y < 8;
		bool bBottom = cursorPos.y > mainLayer->GetHeight() - 8;

		if (bTop && bLeft)
		{
			return HTTOPLEFT;
		}
		else if (bTop && bRight)
		{
			return HTTOPRIGHT;
		}
		else if (bBottom && bRight)
		{
			return HTBOTTOMRIGHT;
		}
		else if (bBottom && bLeft)
		{
			return HTBOTTOMLEFT;
		}
		else if (bLeft)
		{
			return HTLEFT;
		}
		else if (bRight)
		{
			return HTRIGHT;
		}
		else if (bTop)
		{
			return HTTOP;
		}
		else if (bBottom)
		{
			return HTBOTTOM;
		}
		else if (closeBtn && closeBtn->IsCursorInside())
		{
			// HTNOWHERE
		}
		else if (maximizeBtn && maximizeBtn->IsCursorInside())
		{
			// HTNOWHERE
		}
		else if (minimizeBtn && minimizeBtn->IsCursorInside())
		{
			// HTNOWHERE
		}
		else if (captionBar && captionBar->IsCursorInside())
		{
			return HTCAPTION;
		}
		else
		{
			return HTCLIENT;
		}

		break;
	}
	}

	if (fCallDWP)
		return DefWindowProc(handle, msg, wParam, lParam);

	return 0;
}

} // namespace inl