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
using namespace inl::gxeng;
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
	Core* core;
	GuiEngine* guiE;
	GraphicsEngine* graphicsE;

	bool bWndMaximized;

	Window* wnd;
	Window* gameWnd;

	GuiLayer* mainLayer;

	Gui* captionBar;
	GuiButton* minimizeBtn;
	GuiButton* maximizeBtn;
	GuiButton* closeBtn;
};

Editor::Editor()
{
	bWndMaximized = false;

	core = new Core();

	// Create main window for Editor
	WindowDesc d;
	d.clientSize = Vector2u(800, 600);
	d.style = eWindowStyle::DEFAULT;
	d.userWndProc = std::bind(&Editor::WndProc, this, _1, _2, _3, _4);
	wnd = new Window(d);

	// Create secondary window for GAME inside Editor
	d.clientSize = Vector2u(150, 150);
	d.style = eWindowStyle::BORDERLESS;
	d.userWndProc = nullptr;
	gameWnd = new Window(d);

	HWND editorHwnd = (HWND)wnd->GetHandle();
	HWND gameHwnd = (HWND)gameWnd->GetHandle();
	EnableWindow(gameHwnd, false);
	SetParent(gameHwnd, editorHwnd);
	
	// Resize window, non client area removal made it's size wrong
	wnd->SetRect({ 0,0 }, { 800, 600 });

	//DWORD style = GetWindowLong(b, GWL_STYLE); //get the b style
	//style &= ~(WS_POPUP | WS_CAPTION); //reset the "caption" and "popup" bits
	//style |= WS_CHILD; //set the "child" bit
	//SetWindowLong(b, GWL_STYLE, style); //set the new style of b
	//RECT rc; //temporary rectangle
	////GetClientRect(a, &rc); //the "inside border" rectangle for a
	////MoveWindow(b, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, true); //place b at (x,y,w,h) in a
	//UpdateWindow(a);

	// Init Graphics Engine
	graphicsE = core->InitGraphicsEngine(gameWnd->GetClientWidth(), gameWnd->GetClientHeight(), (HWND)gameWnd->GetHandle());

	// Init Gui Engine
	guiE = core->InitGuiEngine(graphicsE, wnd);

	// Init Gui
	InitGui();

	HANDLE hIcon = LoadImage(0, L"InlineEngineLogo.ico", IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
	if (hIcon)
	{
		//Change both icons to the same icon handle.
		SendMessage((HWND)wnd->GetHandle(), WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		SendMessage((HWND)wnd->GetHandle(), WM_SETICON, ICON_BIG, (LPARAM)hIcon);

		//This will ensure that the application icon gets changed too.
		SendMessage(GetWindow((HWND)wnd->GetHandle(), GW_OWNER), WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		SendMessage(GetWindow((HWND)wnd->GetHandle(), GW_OWNER), WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	}
}

Editor::~Editor()
{
	delete core;

	wnd->Close();
	delete wnd;
	delete gameWnd;
}

void Editor::InitGui()
{
	// New Layer
	mainLayer = guiE->AddLayer();

	// Layer border
	mainLayer->SetBorder(1, Color(100));

	// Main layout of the editor is a simple list
	GuiList* mainLayout = mainLayer->AddList();
	mainLayout->StretchFillParent(); // Fill the layer
	mainLayout->SetDirection(eGuiDirection::VERTICAL);
	mainLayout->SetBgToColor(Color::BLACK);

	// Caption bar
	captionBar = mainLayer->AddGui();
	captionBar->SetBgToColor(Color(45));
	captionBar->SetRect(0, 0, 100, 26);

	// Minimize, Maximize, Close btn
	GuiList* minMaxCloseList = mainLayer->AddList();
	minMaxCloseList->StretchFitToChildren();
	minimizeBtn = mainLayer->AddButton();
	maximizeBtn = mainLayer->AddButton();
	closeBtn = mainLayer->AddButton();

	minimizeBtn->onMouseClicked += [this](CursorEvent& evt) { wnd->MinimizeSize(); };
	maximizeBtn->onMouseClicked += [this](CursorEvent& evt)
	{
		if (bWndMaximized)
			wnd->RestoreSize();
		else
			wnd->MaximizeSize();
	};
	closeBtn->onMouseClicked += [this](CursorEvent& evt) { wnd->Close(); };

	minimizeBtn->InitFromImage("Resources/minimize.png","Resources/minimize_h.png");
	maximizeBtn->InitFromImage("Resources/maximize.png","Resources/maximize_h.png");
	closeBtn->InitFromImage("Resources/close.png", "Resources/close_h.png");

	minMaxCloseList->SetDirection(eGuiDirection::HORIZONTAL);
	minMaxCloseList->Add(minimizeBtn);
	minMaxCloseList->Add(maximizeBtn);
	minMaxCloseList->Add(closeBtn);
	minMaxCloseList->AlignRight();

	// Editor caption text
	GuiText* inlineEngineText = mainLayer->AddText();
	inlineEngineText->SetFontSize(14);
	inlineEngineText->SetFontStyle(Gdiplus::FontStyle::FontStyleBold);
	inlineEngineText->SetText("Inline Editor");
	inlineEngineText->AlignCenterVer();
	inlineEngineText->StretchHorFillParent();
	inlineEngineText->SetMarginLeft(7);

	captionBar->Add(inlineEngineText);
	captionBar->Add(minMaxCloseList);
	captionBar->StretchHorFillParent();
	captionBar->SetPos(0, 1);
	//captionBar->SetBorder(0, 0, 0, 1, Color(70));

	mainLayout->Add(captionBar);

	// Main menu bar
	GuiMenu* menuBar = mainLayer->AddMenu();
	menuBar->SetBorder(0, 0, 0, 1, Color(70));
	menuBar->SetDirection(eGuiDirection::HORIZONTAL);
	menuBar->SetBgColorForAllStates(Color(25));
	menuBar->SetRect(1, captionBar->GetHeight(), 400, 400);
	menuBar->StretchHorFillParent();
	menuBar->StretchVerFitToChildren();
	{
		GuiMenu* fileMenu = menuBar->AddItemMenu("File");
		GuiMenu* buildMenu = menuBar->AddItemMenu("Build");
		GuiMenu* toolsMenu = menuBar->AddItemMenu("Tools");
		GuiMenu* helpMenu = menuBar->AddItemMenu("Help");
		fileMenu->SetBorder(1, Color(70));
		buildMenu->SetBorder(1, Color(70));
		toolsMenu->SetBorder(1, Color(70));
		helpMenu->SetBorder(1, Color(70));

		fileMenu->AddItemButton("New Scene");
		fileMenu->AddItemButton("Open Scene");
		Gui* separator0 = fileMenu->AddItemSeparatorHor();
		separator0->SetBgToColor(Color(70));
		fileMenu->AddItemButton("Save Scene");
		fileMenu->AddItemButton("Save Scene as...");
		Gui* separator1 = fileMenu->AddItemSeparatorHor();
		separator1->SetBgToColor(Color(70));
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
		toolsMenu->AddItemButton("** PUT TOOLS HERE **");

		helpMenu->AddItemButton("About Inline Engine");

		for (auto& child : fileMenu->GetItems()) { child->SetBgToColor(Color(25), Color(65)); }
		for (auto& child : buildMenu->GetItems()) { child->SetBgToColor(Color(25), Color(65)); }
		for (auto& child : toolsMenu->GetItems()) { child->SetBgToColor(Color(25), Color(65)); }
		for (auto& child : helpMenu->GetItems()) { child->SetBgToColor(Color(25), Color(65)); }

		//GuiButton* btn = menuBar->AddButton();
		//btn->SetText("File");
		//GuiButton* btn1 = menuBar->AddButton();
		//btn1->SetText("Edit");
		//GuiButton* btn2 = menuBar->AddButton();
		//btn2->SetText("Tools");
		//GuiButton* btn3 = menuBar->AddButton();
		//btn3->SetText("Help");

		for (Gui* c : menuBar->GetChildren())
		{
			c->SetBgToColor(Color(25), Color(65));
			c->StretchFitToChildren();
			c->SetPadding(4);
		}
	}
	mainLayout->Add(menuBar);

	GuiSplitter* split0 = mainLayer->AddSplitter(); // split main
	GuiSplitter* split1 = mainLayer->AddSplitter(); // split main left to top, bottom
	GuiSplitter* split2 = mainLayer->AddSplitter(); // split main left-top to left, right
	split0->Stretch(eGuiStretch::FILL_PARENT_POSITIVE_DIR, eGuiStretch::FILL_PARENT_POSITIVE_DIR);
	split1->StretchFillParent();
	split2->StretchFillParent();
	
	split0->SetDirection(eGuiDirection::HORIZONTAL);
	split1->SetDirection(eGuiDirection::VERTICAL);
	split2->SetDirection(eGuiDirection::HORIZONTAL);

	split0->SetSize(400, 400);
	split1->SetSize(200, 400);

	Gui* area0 = mainLayer->AddButton();
	Gui* area1 = mainLayer->AddButton();
	Gui* area2 = mainLayer->AddButton();
	Gui* area3 = mainLayer->AddButton();
	area0->SetSize(100, 100);
	area0->SetBgToColor(Color(35));
	area1->SetSize(100, 100);
	area1->SetBgToColor(Color(35));
	area2->SetSize(100, 100);
	area2->SetBgToColor(Color(35));
	area3->SetSize(100, 100);
	area3->SetBgToColor(Color(0));
	area3->StretchFillParent();

	area3->onSizeChangedClonable += [this](Gui* self, Vector2f size)
	{
		HWND gameHwnd = (HWND)gameWnd->GetHandle();
		HWND editorHwnd = (HWND)this->wnd->GetHandle();
		SetWindowPos(gameHwnd, NULL, self->GetContentPosX(), self->GetContentPosY(), self->GetContentSizeX(), self->GetContentSizeY(), 0);
		SetFocus(editorHwnd);
	};

	area0->StretchFillParent();
	area1->StretchFillParent();
	area2->StretchFillParent();

	// Split main area to left and right
	menuBar->RefreshLayout();
	split0->SetPos(menuBar->GetPosBottomLeft());
	
	split0->AddItem(split1);
	split0->AddItem(area0);

	//split1->AddItem(area1);
	split1->AddItem(split2);
	split1->AddItem(area1);

	split2->AddItem(area2);
	split2->AddItem(area3);
	mainLayout->Add(split0);

	//GuiList* list = area2->AddList();
	//auto btn0 = list->AddItemButton("Nesze LOL");
	//auto btn1 = list->AddItemButton("--Xbox One...");
	//btn0->StretchHorFillParent();
	//btn1->StretchHorFillParent();
	//list->StretchFitToChildren();
	//list->SetPos(30, 30);
}

void Editor::Run()
{
	// Create timer, delta time -> engine
	Timer* timer = new Timer();
	timer->Start();

	wnd->SetTitle("Inline Editor");

	// Editor main loop
	while (wnd->IsOpen())
	{
		// Prepare for input processing
		//Input.ClearFrameData();

		WindowEvent evt;
		while (wnd->PopEvent(evt));
		while (gameWnd->PopEvent(evt));

		// Dispatch Inputs
		//Input.Update();

		// Frame delta time
		float deltaTime = timer->Elapsed();

		// Update engine
		core->Update(deltaTime);

		//Sleep(30);
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
			if(!guiE->IsUsingCustomCursor())
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

		if (guiE)
			guiE->Render();

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
				maximizeBtn->InitFromImage("Resources/restore.png", "Resources/restore_h.png");
				bWndMaximized = true;
			}
			else if (wParam == SIZE_RESTORED)
			{
				maximizeBtn->InitFromImage("Resources/maximize.png", "Resources/maximize_h.png");
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
		Vector2f cursorPos = guiE->GetCursorPos();
		
		bool bLeft =	cursorPos.x() < 8;
		bool bRight =	cursorPos.x() > mainLayer->GetWidth() - 8;
		bool bTop =		cursorPos.y() < 8;
		bool bBottom =	cursorPos.y() > mainLayer->GetHeight() - 8;

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