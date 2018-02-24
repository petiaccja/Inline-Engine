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
	wnd = new Window("Inline Engine", Vec2u(800, 600), true, true, false);

	wnd->OnPaint += [this]()
	{
		if (guiEngine)
			guiEngine->Render();
	};

	wnd->OnResize += [this](ResizeEvent& e)
	{
		if (maximizeBtn)
		{
			if (e.resizeMode == eResizeMode::MAXIMIZED)
			{
				maximizeBtn->SetImages(L"Resources/restore.png", L"Resources/restore_h.png");
				bWndMaximized = true;
			}
			else if (e.resizeMode == eResizeMode::RESTORED)
			{
				maximizeBtn->SetImages(L"Resources/maximize.png", L"Resources/maximize_h.png");

				bWndMaximized = false;
			}
		}
	};

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
	ListView* mainLayout = mainLayer->AddGui<ListView>();
	mainLayout->StretchFillParent(); // Fill the layer
	mainLayout->SetOrientation(eGuiOrientation::VERTICAL);
	mainLayout->SetBgToColor(ColorI(0, 0, 0, 255));
	
	// Caption bar
	captionBar = mainLayer->AddGui();
	captionBar->SetBgToColor(ColorI(45, 45, 45, 255));
	captionBar->SetRect(0, 0, 100, 26);
	
	// Minimize, Maximize, Close btn
	ListView* minMaxCloseList = mainLayer->AddGui<ListView>();
	minMaxCloseList->StretchFitToContent();
	minimizeBtn = mainLayer->AddGui<Image>();
	maximizeBtn = mainLayer->AddGui<Image>();
	closeBtn = mainLayer->AddGui<Image>();
	
	minimizeBtn->OnCursorRelease += [this](CursorEvent& evt) { wnd->Minize(); };
	maximizeBtn->OnCursorRelease += [this](CursorEvent& evt)
	{
		if (bWndMaximized)
			wnd->Restore();
		else
			wnd->Maximize();
	};
	closeBtn->OnCursorRelease += [this](CursorEvent& evt) { wnd->Close(); };
	
	minimizeBtn->SetImages(L"Resources/minimize.png", L"Resources/minimize_h.png");
	maximizeBtn->SetImages(L"Resources/maximize.png", L"Resources/maximize_h.png");
	closeBtn->SetImages(L"Resources/close.png", L"Resources/close_h.png");
	
	minMaxCloseList->SetOrientation(eGuiOrientation::HORIZONTAL);
	minMaxCloseList->AddItem(minimizeBtn);
	minMaxCloseList->AddItem(maximizeBtn);
	minMaxCloseList->AddItem(closeBtn);
	minMaxCloseList->AlignRight();
	
	// Editor caption text
	Text* inlineEngineText = mainLayer->AddGui<Text>();
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
	Menu* menuBar = mainLayer->AddGui<Menu>();
	menuBar->SetBorder(0, 0, 0, 1, ColorI(70, 70, 70, 255));
	menuBar->SetOrientation(eGuiOrientation::HORIZONTAL);
	menuBar->SetBgToColor(ColorI(25, 25, 25, 255));
	menuBar->StretchHorFillParent();
	menuBar->StretchVerFitToContent();
	{
		Menu* fileMenu = menuBar->AddItemMenu("File");
		Menu* buildMenu = menuBar->AddItemMenu("Build");
		Menu* toolsMenu = menuBar->AddItemMenu("Tools");
		Menu* helpMenu = menuBar->AddItemMenu("Help");
	
		//fileMenu->GetButtonText()->SetFontSize(18);
		//buildMenu->GetButtonText()->SetFontSize(18);
		//toolsMenu->GetButtonText()->SetFontSize(18);
		//helpMenu->GetButtonText()->SetFontSize(18);
	
        CheckBox* chkBox = fileMenu->AddItem<CheckBox>();
        chkBox->SetText("CheckBox");

		Button* tmpBtn = fileMenu->AddItem<Button>();
		tmpBtn->SetText("New Scene");
	
		tmpBtn = fileMenu->AddItem<Button>();
		tmpBtn->SetText("Open Scene");
		Gui* separator0 = fileMenu->AddItem<Gui>();
		separator0->SetSize(1, 1);
		separator0->StretchHorFillParent();
		separator0->SetBgToColor(ColorI(80, 80, 80, 255));
		separator0->DisableHover();
		tmpBtn = fileMenu->AddItem<Button>();
		tmpBtn->SetText("Save Scene");
		tmpBtn = fileMenu->AddItem<Button>();
		tmpBtn->SetText("Save Scene as...");
		Gui* separator1 = fileMenu->AddItem<Gui>();
		separator1->SetSize(1, 1);
		separator1->StretchHorFillParent();
		separator1->SetBgToColor(ColorI(80, 80, 80, 255));
		separator1->DisableHover();
		tmpBtn = fileMenu->AddItem<Button>();
		tmpBtn->SetText("New Project...");
		tmpBtn = fileMenu->AddItem<Button>();
		tmpBtn->SetText("Open Project...");
		tmpBtn = fileMenu->AddItem<Button>();
		tmpBtn->SetText("Save Project");
	
		tmpBtn = buildMenu->AddItem<Button>();
		tmpBtn->SetText("Windows...");
		tmpBtn = buildMenu->AddItem<Button>();
		tmpBtn->SetText("Linux...");
		tmpBtn = buildMenu->AddItem<Button>();
		tmpBtn->SetText("Mac...");
		tmpBtn = buildMenu->AddItem<Button>();
		tmpBtn->SetText("XBox One...");
		tmpBtn = buildMenu->AddItem<Button>();
		tmpBtn->SetText("PS4...");
		tmpBtn = buildMenu->AddItem<Button>();
		tmpBtn->SetText("Android");
		tmpBtn = buildMenu->AddItem<Button>();
		tmpBtn->SetText("IOS");
	
		Menu* menu0 = toolsMenu->AddItemMenu("TESZT - 0");
		Menu* menu1 = menu0->AddItemMenu("TESZT - 1");
		Menu* menu2 = menu1->AddItemMenu("TESZT - 2");
	
		Menu* menu00 = toolsMenu->AddItemMenu("TESZT - 00");
		Menu* menu01 = menu00->AddItemMenu("TESZT - 01");
		Menu* menu001 = menu00->AddItemMenu("TESZT - 001");
		Menu* menu02 = menu01->AddItemMenu("TESZT - 02");
		Menu* menu002 = menu001->AddItemMenu("TESZT - 002");
		tmpBtn = toolsMenu->AddItem<Button>();
		tmpBtn->SetText("** PUT TOOLS HERE **");
	
		tmpBtn = helpMenu->AddItem<Button>();
		tmpBtn->SetText("About Inline Engine");
	
	
		for (Gui* c : { fileMenu, buildMenu, toolsMenu, helpMenu, menu0, menu1, menu00, menu01, menu001, menu02, menu002 })
		{
			c->SetBorder(1, ColorI(70, 70, 70, 255));
			for (Gui* child : c->GetChildrenRecursive<Button>())
			{
				child->SetBgToColor(ColorI(25, 25, 25, 255), ColorI(65, 65, 65, 255));
				child->SetPadding(2, 2, 4, 4);
			}
		}
	
		for (Gui* c : menuBar->GetChildrenRecursive<Button>())
		{
			c->SetBgToColor(ColorI(25, 25, 25, 255), ColorI(65, 65, 65, 255));
			c->StretchFitToContent();
			c->SetPadding(4, 4, 2, 2);
			c->AlignCenter();
		}
	}
	mainLayout->AddItem(menuBar);
	
	//Gui* toolBar = mainLayer->AddGui();
	//toolBar->StretchHorFillParent();
	//toolBar->SetBgToColor(ColorI(25));
	//mainLayout->AddItem(toolBar);
	
	Splitter* split0 = mainLayer->AddGui<Splitter>(); // split main
	Splitter* split1 = mainLayer->AddGui<Splitter>(); // split main left to (top, bottom)
	Splitter* split2 = mainLayer->AddGui<Splitter>(); // split main left-top to (left, right)
	split0->Stretch(eGuiStretch::FILL_SPACE_POSITIVE_DIR, eGuiStretch::FILL_SPACE_POSITIVE_DIR);
	split1->StretchFillParent();
	split2->StretchFillParent();
	
	split0->SetOrientation(eGuiOrientation::HORIZONTAL);
	split1->SetOrientation(eGuiOrientation::VERTICAL);
	split2->SetOrientation(eGuiOrientation::HORIZONTAL);
	
	//split0->SetSize(400, 400);
	split1->SetSize(750, 400);
	split2->SetSize(750, 300);
	
	Gui* rightArea = mainLayer->AddGui<Button>();
	Gui* bottomArea = mainLayer->AddGui<Button>();
	Gui* leftArea = mainLayer->AddGui<Button>();
	centerRenderArea = mainLayer->AddGui<Button>();
	rightArea->SetSize(150, 100);
	rightArea->SetBgToColor(ColorI(10, 10, 10, 255));
	bottomArea->SetSize(100, 100);
	bottomArea->SetBgToColor(ColorI(10, 10, 10, 255));
	leftArea->SetSize(30, 100);
	leftArea->SetBgToColor(ColorI(10, 10, 10, 255));
	centerRenderArea->SetSize(100, 100);
	centerRenderArea->SetBgToColor(ColorI(0, 0, 0, 255));
	centerRenderArea->StretchFillParent();
	
	centerRenderArea->OnCursorClick += [this](CursorEvent& evt)
	{
		//if (evt.mouseButton == eMouseButton::LEFT)
		//{
		//	Ray3D ray = cam->ScreenPointToRay(centerRenderArea->GetCursorPosContentSpace());
		//	TraceResult traceResult;
		//
		//	Vec3 spawnPos;
		//	if (scene->TraceGraphicsRay(ray, traceResult))
		//	{
		//		spawnPos = traceResult.pos;
		//	}
		//	else
		//	{
		//		spawnPos = cam->GetPos() + cam->GetFrontDir() * 200;
		//	}
		//
		//	// Spawn something at that position
		//	RigidBodyActor* actor = scene->AddActor_RigidBody("D:/sphere2.fbx", 0);
		//
		//	actor->SetPos(spawnPos);
		//}
	};
	
	centerRenderArea->OnTransformChange += [this](TransformEvent& e)
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
				separator->SetBorder(1, 1, 0, 0, ColorI(0, 0, 0, 255));
			else
				separator->SetBorder(0, 0, 1, 1, ColorI(0, 0, 0, 255));
		}
	}
	
	ScrollableArea* scrollableBottom = bottomArea->AddGui<ScrollableArea>();
	scrollableBottom->StretchFillParent();
	
	
	ListView* textureList = new ListView(scrollableBottom->guiEngine);
	scrollableBottom->SetContent(textureList);
	
	textureList->SetBgToColor(ColorI(0, 0, 0, 0));
	textureList->SetOrientation(eGuiOrientation::HORIZONTAL);
	textureList->StretchFitToContent();
	
	Gui* contentCell = scrollableBottom->GetCell(0, 0);
	thread_local float colorDiff = 0;
	thread_local float time = 0;
	contentCell->OnOperSysDragEnter += [contentCell](gui::DragDropEvent& data)
	{
		time = 0;
		// Show tooltip about what will happen if user Drops it
	};
	
	contentCell->OnOperSysDragLeave += [contentCell](gui::DragDropEvent& data)
	{
		// Remove the highlight
		contentCell->SetBgActiveColor(contentCell->GetBgIdleColor());
	
		// Hide tooltip
	};
	
	contentCell->OnOperSysDragHover += [contentCell](gui::DragDropEvent& data)
	{
		const float maxColorDifference = 40;
	
		time += Time.deltaTime * 5;
		int color = (int)(sin(time) * maxColorDifference);
		contentCell->SetBgActiveColor(contentCell->GetBgIdleColor() + ColorI(color, color, color, 255));
	};
	
	contentCell->OnOperSysDrop += [this, textureList, contentCell](gui::DragDropEvent& e)
	{
		contentCell->SetBgActiveColor(contentCell->GetBgIdleColor());
	
		std::vector<path> filesPaths = e.filePaths;
		std::string text = e.text;
	
		for (int i = 0; i < filesPaths.size(); ++i)
		{
			path filePath = filesPaths[i];
	
			// Texture image
			ListView* listItem = textureList->AddItem<ListView>();
			listItem->StretchFitToContent();
			listItem->MakeVertical();
			listItem->SetSize(70, 100);
			listItem->SetBgToColor(ColorI(0, 0, 0, 0), ColorI(20, 20, 20, 255));
			Image* img0 = listItem->AddItem<Image>();
			img0->AlignHorCenter();
			img0->SetMargin(4, 4, 4, 0);
			img0->SetImage(filePath.c_str(), 70, 70);
			img0->SetSize(70, 70);
	
			listItem->OnCursorClick += [filePath, this](CursorEvent& evt)
			{
				if (filePath.extension() == L".fbx")
				{
					scene->AddActor_Mesh(filePath);
				}
			};
	
			filePath.replace_extension("");
			std::wstring nameWithoutExt = filePath.filename();
	
			// Texture text
			Text* text0 = listItem->AddGui<Text>();
			text0->StretchFitToContent();
			text0->AlignHorCenter();
			text0->SetMargin(4);
			text0->SetText(nameWithoutExt);
			listItem->DisableChildrenHover();
		}
	
		SetFocus((HWND)wnd->GetNativeHandle());
	};
	
	ListView* options = rightArea->AddGui<ListView>();
	options->SetOrientation(eGuiOrientation::VERTICAL);
	options->StretchHorFillParent();
	//options->StretchVerFitToContent();
	options->StretchVerFillParent();
	options->SetBgToColor(ColorI(0, 0, 0, 0));
	options->SetName("__OPTIONS__");
	
	Collapsable* dof = options->AddGui<Collapsable>();
	//dof->SetBorder(1, ColorI(80, 80, 80, 255));
	dof->SetCaptionText(L"Depth of Field");
	
	Collapsable* ssao = options->AddItem<Collapsable>();
	//ssao->SetMargin(1);
	//ssao->SetBorder(1, ColorI(80, 80, 80, 255));
	ssao->SetCaptionText(L"SSAO");
	
	Collapsable* voxelGI = options->AddItem<Collapsable>();
	////voxelGI->SetMargin(1);
	////voxelGI->SetBorder(1, ColorI(80, 80, 80, 255));
	voxelGI->SetCaptionText(L"VoxelGI");
	//
	Collapsable* ssr = options->AddItem<Collapsable>();
	////ssr->SetMargin(1);
	////ssr->SetBorder(1, ColorI(80, 80, 80, 255));
	ssr->SetCaptionText(L"SSR");
	
	Button* tmp = dof->AddItem<Button>();
	tmp->SetText("parameter 0");
	tmp = dof->AddItem<Button>();
	tmp->SetText("parameter 1");
	tmp = dof->AddItem<Button>();
	tmp->SetText("parameter 2");
	tmp = dof->AddItem<Button>();
	tmp->SetText("parameter 3");
	tmp = dof->AddItem<Button>();
	tmp->SetText("parameter 4");
	
	tmp = ssao->AddItem<Button>();
	tmp->SetText("parameter 0");
	tmp = ssao->AddItem<Button>();
	tmp->SetText("parameter 1");
	
	tmp = voxelGI->AddItem<Button>();
	tmp->SetText("parameter 0");
	tmp = voxelGI->AddItem<Button>();
	tmp->SetText("parameter 1");
	
	tmp = ssr->AddItem<Button>();
	tmp->SetText("parameter 0");
	tmp = ssr->AddItem<Button>();
	tmp->SetText("parameter 1");

	// RAW GRID TEST
	//Grid* grid = mainLayer->AddGui<Grid>();
	//grid->StretchHorFillParent();
	//grid->StretchVerFitToContent();
	//grid->SetDimension(2, 2);
	//
	//grid->GetColumn(0)->SetWidth(100);
	//grid->GetColumn(1)->StretchFillSpace(1.0);
	//
	//grid->GetRow(0)->SetHeight(100);
	//grid->GetRow(1)->StretchFitToContent();
	//
	//Button* btn0 = grid->GetCell(0, 0)->AddGui<Button>();
	//Button* btn1 = grid->GetCell(1, 0)->AddGui<Button>();
	//Button* btn2 = grid->GetCell(0, 1)->AddGui<Button>();
	//
	//btn1->SetText("Depth of Field (so long name Depth of Field)");
	//
	//btn0->StretchFillParent();
	//btn1->StretchFillParent();
	//btn2->StretchFillParent();
	//
	//btn0->SetBgToColor(ColorI(255, 0, 0, 255));
	//btn1->SetBgToColor(ColorI(0, 0, 255, 255));
	//btn2->SetBgToColor(ColorI(0, 255, 0, 255));
	//
	//ListView* list = grid->GetCell(1, 1)->AddGui<ListView>();
	//Button* tmp = list->AddItem<Button>(); tmp->SetText("parameter0");	tmp->StretchHorFillParent();
	//tmp = list->AddItem<Button>(); tmp->SetText("parameter1");				tmp->StretchHorFillParent();
	//tmp = list->AddItem<Button>(); tmp->SetText("parameter2");				tmp->StretchHorFillParent();
	//tmp = list->AddItem<Button>(); tmp->SetText("parameter3");				tmp->StretchHorFillParent();
	//tmp = list->AddItem<Button>(); tmp->SetText("parameter4");				tmp->StretchHorFillParent();
	//tmp = list->AddItem<Button>(); tmp->SetText("parameter5");				tmp->StretchHorFillParent();
	//tmp = list->AddItem<Button>(); tmp->SetText("parameter6");				tmp->StretchHorFillParent();
	//tmp = list->AddItem<Button>(); tmp->SetText("parameter7");				tmp->StretchHorFillParent();
}

void Editor::Update()
{
	// Create timer, delta time -> engine
	Timer* timer = new Timer();
	timer->Start();

	wnd->SetTitle("Inline Editor");

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
		//if(centerRenderArea->IsCursorInside())
		//	cam->Update(Time.deltaTime);

		// Update game world
		//world->UpdateWorld(Time.deltaTime);
	
		// Update engine
		core->Update(Time.deltaTime);
	}

	delete timer;
}
/*
LRESULT Editor::WndProc(HWND handle, UINT msg, WPARAM wParam, LPARAM lParam)
{
	bool fCallDWP = !DwmDefWindowProc(handle, msg, wParam, lParam, NULL);

	switch (msg)
	{
		case WM_SETCURSOR:
		{
			if (LOWORD(lParam) == HTCLIENT)
			{
				if (!guiEngine->IsUsingCustomCursor())
					SetCursor(LoadCursor(nullptr, IDC_ARROW));
		
				return TRUE;
			}
			break;
		}
		case WM_NCCALCSIZE:
		{
			// Calculate new NCCALCSIZE_PARAMS based on custom NCA inset.
			NCCALCSIZE_PARAMS *pncsp = reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);
		
			if (IsMaximized(handle))
			{
				pncsp->rgrc[0].left = pncsp->rgrc[0].left + 8;
				pncsp->rgrc[0].top = pncsp->rgrc[0].top + 8;
				pncsp->rgrc[0].right = pncsp->rgrc[0].right - 8;
				pncsp->rgrc[0].bottom = pncsp->rgrc[0].bottom - 8;
			}
			else
			{
				pncsp->rgrc[0].left = pncsp->rgrc[0].left + 0;
				pncsp->rgrc[0].top = pncsp->rgrc[0].top + 0;
				pncsp->rgrc[0].right = pncsp->rgrc[0].right - 0;
				pncsp->rgrc[0].bottom = pncsp->rgrc[0].bottom - 0;
			}
		
			// No need to pass the message on to the DefWindowProc.
			fCallDWP = false;
		
			break;
		}
		case WM_NCHITTEST:
		{
			Vec2 cursorPos = guiEngine->GetCursorPos();
			
			int border;
			if (IsMaximized(handle))
				border = 0;
			else
				border = 3;
			
			bool bLeft = cursorPos.x < border;
			bool bRight = cursorPos.x > mainLayer->GetWidth() - border;
			bool bTop = cursorPos.y < border;
			bool bBottom = cursorPos.y > mainLayer->GetHeight() - border;
			
			GuiRectF captionBarRect = captionBar->GetRect();
			captionBarRect.MoveSides(GuiRectF(-1, 1, 0, -1));
		
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
			else if (captionBar && captionBarRect.IsPointInside(guiEngine->GetCursorPos()))
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
*/

} // namespace inl