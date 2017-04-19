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
	SetWindowPos(gameHwnd, editorHwnd, 300, 300, 150, 150, 0);
	SetParent(gameHwnd, editorHwnd);
	
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

	// Resize window, non client area removal made it's size wrong
	wnd->SetRect({ 0,0 }, { 800, 600 });
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

	mainLayer->SetBorder(1, Color(120));

	//GuiText* t = mainLayer->AddText();
	//t->SetText(L"Nem igaz már hogy nem mûködik");
	//t->SetBgColorForAllStates(Color::RED);
	//t->SetRect(30, 130, 150, 40);
	////t->AlignFitChildrenHor();
	//
	//GuiText* t2 = t->Clone();
	//GuiText* t3 = t->Clone();
	//t2->SetRect(50, 180, 150, 40);
	//t3->SetRect(30, 230, 150, 40);
	////t2->AlignFillParentHor(); // RICSI NEM MUKODIK akkor ha "container->AlignFitChildren()" meg van hívva
	//
	//t2->SetName("T2");
	//t3->SetName("T3");
	////t2->AlignFillParentHor();
	////t3->AlignFillParentHor(); // RICSI NEM MUKODIK akkor ha "container->AlignFitChildren()" meg van hívva
	////t2->AlignRight();
	//// RICSI AlignLeft() nem kéne hogy declineolja a FitChildren - t...
	//
	//Gui* container2 = mainLayer->AddGui();
	//container2->SetRect(50, 180, 160, 40);
	//container2->SetBgColorForAllStates(Color::GREEN);
	//container2->SetName("Green");
	//container2->Add(t2);
	//
	////container2->AlignFitChildrenHor();
	////container2->AlignFillParentHor();
	////t3->AlignFitChildrenHor();
	//// RICSI !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Ennek nem szabadna a
	////t3->AlignLeft();
	////t->AlignCenter();
	////t->SetAlign(eGuiAlign::CENTER);
	////GuiText* t2 = t->Clone();
	//
	////t->SetAlign(eGuiAlign::STRETCH_H);
	////t->AlignStretchHorizontal();
	////t->SetAutoWidth(true);
	//
	////t2->SetRect(100, 140, 100, 40);
	//
	//Gui* container = mainLayer->AddGui();
	//container->SetRect(0, 100, 500, 200);
	//container->SetBgColorForAllStates(Color::BLUE);
	//container->SetName("Blue");
	//container->Add(t);
	//container->Add(container2);
	//container->Add(t3);
	////container->AlignFitChildren();
	////container->AlignFitChildrenHor();
	////container->Add(t2);
	//
	//Gui* container3 = mainLayer->AddGui();
	//container3->SetRect(0, 100, 700, 450);
	//container3->SetBgColorForAllStates(Color::WHITE);
	//container3->Add(container);
	//
	////t2->StretchFillParentHor();
	//t3->StretchFillParentHor();
	////container2->StretchFillParentHor();
	//container->StretchFitToChildrenHor();
	////container->StretchFillParentVer();
	////container3->StretchFitToChildrenHor();
	//
	//return;
	//GuiButton* text = mainLayer->AddButton();
	//text->SetText("LOLMAR_HAT_NEM_IGAZ");
	//text->SetAutoSize(true);
	//text->SetBgColorForAllStates(Color::RED);
	////text->SetColor(Color(255)); // TODO !!!
	//text->SetRect(100, 100, 10, 10);
	// Arrange Measure test

	//GuiButton* btn2 = mainLayer->AddButton();
	//btn2->SetText(L"Button");
	////btn2->SetBgColorForAllStates(Color(0));
	//btn2->GetTextGui()->SetBgColorForAllStates(Color(50));
	//btn2->GetTextGui()->SetAlign(eGuiAlign::STRETCH);
	//btn2->SetBorder(3, Color::RED);
	//btn2->SetPadding(10);
	//
	//btn2->SetRect(0, 0, 100, 200);
	//btn2->SetContentAlign(eGuiAlign::CENTER);

	// Arrange Measure test

	//GuiList* list = mainLayer->AddList();
	//GuiButton* btn = list->AddButton();
	//btn->SetText("Button1asdasdasd");
	//btn->SetMargin(5, 5, 5, 5);
	////btn->SetPadding(4);
	//////btn->SetAutoWidth(true);
	////btn->SetBorder(1, Color(100));
	////btn->SetAlign(eGuiAlign::STRETCH_H);
	////btn->SetSize(btn->GetSize() + Vector2f(20, 20));
	//GuiButton* btn2 = list->AddButton();
	//btn2->SetText("Button2");
	//btn2->SetMargin(0, 0, 0, 0);
	////btn2->SetPadding(4);
	////btn2->SetBorder(1, Color(100));
	//list->SetBgColorForAllStates(Color(150));
	//list->SetRect(0, 0, 100, 100);
	//return;


	//GuiList* list = mainLayer->AddList();
	//GuiButton* btn0 = list->AddButton();
	//btn0->SetText("Teszt");
	//GuiButton* btn1 = list->AddButton();
	//btn1->SetText("Teszt2");
	//
	//list->SetRect(100, 100, 200, 200);
	//return;


	//GuiList* list = mainLayer->AddList();
	//GuiButton* btn0 = list->AddButton();
	//GuiButton* btn1 = list->AddButton();
	//btn0->SetText(L"First___");
	//btn1->SetText(L"Second___");
	//
	//list->SetRect(0, 0, 100, 200);
	// Gui collapsable
	//{
	//	GuiCollapsable* collapsable = mainLayer->AddCollapsable();
	//	collapsable->SetName("collapsable");
	//	collapsable->SetCaptionText(L"Collapsable");
	//	
	//	collapsable->AddToList<GuiButton>()->SetText("LOL");
	//	collapsable->AddToList<GuiButton>()->SetText("LOL2");
	//	collapsable->SetRect(100, 100, 200, 30);
	//	GuiCollapsable* collapsable2 = mainLayer->AddCollapsable();
	//	collapsable2->SetName("Collapsable2");
	//	collapsable2->SetCaptionText(L"Collapsable2");
	//	collapsable2->SetRect(500, 520, 60, 30);
	//	collapsable2->AddToList<GuiButton>()->SetText("LOL");
	//	collapsable2->AddToList<GuiButton>()->SetText("LOL2");
	//	
	//	GuiCollapsable* collapsableList = mainLayer->AddCollapsable();
	//	collapsableList->SetCaptionText(L"CaptionText");
	//	collapsableList->SetName("CollapsableList");
	//	collapsableList->AddToList(collapsable);
	//	collapsableList->AddToList(collapsable2);
	//	//collapsableList->SetFitToChildren(true);
	//	collapsableList->SetRect(200, 200, 10, 10);
	//	Gui* caption = collapsableList->GetCaption();
	//	//caption->SetBgIdleColor(Color(20));
	//	//caption->SetBgHoverColor(Color(0));
	//
	//
	//	int asd = 5;
	//	asd++;
	//}



	//auto list = mainLayer->AddList();
	//auto btn0 = list->AddButton();
	//auto btn1 = list->AddButton();
	//btn0->SetText(L"short");
	//btn1->SetText(L"long log long long long");
	//list->SetRect(200, 200, 100, 200);


	//auto list = mainLayer->AddList();
	//auto btn0 = list->AddButton();
	//auto btn1 = list->AddButton();
	//btn0->SetText(L"LOL");
	//btn1->SetText(L"LOL2e12321");
	//list->SetBorder(1, Color::RED);
	//btn0->SetBorder(1, Color::GREEN);
	//btn0->SetName("GREEN");
	//btn1->SetBorder(1, Color::BLUE);
	//btn1->SetName("BLUE");
	//
	//auto list2 = list->Clone();
	//
	//list->Add(list2);
	//
	//list->SetRect(200, 200, 300, 300);
	//return;

	//mainLayer->SetBorder(10, Color(120));

	// Caption bar
	captionBar = mainLayer->AddGui();
	captionBar->SetBgToColor(Color(43), Color(43));
	captionBar->SetRect(0, 0, 100, 26);

	//static bool bDragging = false;
	//static Vector2i mousePosWhenPressed;
	//static RECT rectWhenPressed;
	//
	//captionBar->onMousePressed += [](CursorEvent& evt)
	//{
	//	bDragging = true;
	//	
	//	GetWindowRect((HWND)window->GetHandle(), &rectWhenPressed);
	//
	//	POINT p;
	//	p.x = evt.cursorContentPos.x();
	//	p.y = evt.cursorContentPos.y();
	//	ClientToScreen((HWND)window->GetHandle(), &p);
	//	mousePosWhenPressed.x() = p.x;
	//	mousePosWhenPressed.y() = p.y;
	//};
	//
	//captionBar->onMouseMoved += [](CursorEvent& evt)
	//{
	//	
	//};
	//
	//captionBar->onUpdate += [](float deltaTime)
	//{
	//	if (GetKeyState(VK_LBUTTON) > 0)
	//	{
	//		bDragging = false;
	//	}
	//
	//	if (bDragging)
	//	{
	//		//Vector2i mouseDelta = evt.mouseDelta;
	//
	//		RECT finalRect = rectWhenPressed;
	//
	//		Vector2i mousePos;
	//		POINT p;
	//		//p.x = evt.cursorContentPos.x();
	//		//p.y = evt.cursorContentPos.y();
	//		//ClientToScreen((HWND)window->GetHandle(), &p);
	//		GetCursorPos(&p);
	//		mousePos.x() = p.x;
	//		mousePos.y() = p.y;
	//
	//		finalRect.left += mousePos.x() - mousePosWhenPressed.x();
	//		finalRect.right += mousePos.x() - mousePosWhenPressed.x();
	//
	//		finalRect.top += mousePos.y() - mousePosWhenPressed.y();
	//		finalRect.bottom += mousePos.y() - mousePosWhenPressed.y();
	//
	//		SetWindowPos((HWND)window->GetHandle(), false, finalRect.left, finalRect.top, finalRect.right - finalRect.left, finalRect.bottom - finalRect.top, 0);
	//		UpdateWindow((HWND)window->GetHandle());
	//	}
	//};

	// Minimize, Maximize, Close btn
	GuiList* minMaxCloseList = mainLayer->AddList();
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

	minimizeBtn->InitFromImage(L"Resources/minimize.png", L"Resources/minimize_h.png");
	maximizeBtn->InitFromImage(L"Resources/maximize.png", L"Resources/maximize_h.png");
	closeBtn->InitFromImage(L"Resources/close.png", L"Resources/close_h.png");

	minMaxCloseList->SetDirection(eGuiListDirection::HORIZONTAL);
	minMaxCloseList->Add(minimizeBtn);
	minMaxCloseList->Add(maximizeBtn);
	minMaxCloseList->Add(closeBtn);
	minMaxCloseList->AlignRight();

	// Editor caption text
	GuiText* inlineEngineText = mainLayer->AddText();
	inlineEngineText->SetText(L"Inline Editor");
	inlineEngineText->AlignCenterVer();
	inlineEngineText->StretchFillParentHor();
	inlineEngineText->SetMarginLeft(7);

	captionBar->Add(inlineEngineText);
	captionBar->Add(minMaxCloseList);
	captionBar->StretchFillParentHor();
	captionBar->SetName(L"CAPTION_BAR");
	//Gui* image = mainLayer->AddImage(L"Resources/minimize.png", L"Resources/minimize_h.png");

	//Gui* minimizeBtn = Gui::FromImage(L"Resources/minimize.png", L"Resources/minimize_h.png");
	//mainLayer->AddGui(minimizeBtn);

	//Gui* minimizeBtn = mainLayer->Add(Gui::FromImage(L"Resources/minimize.png", L"Resources/minimize_h.png"));
	//minimizeBtn->SetBgIdleImage(L"Resources/minimize.png");
	//minimizeBtn->SetBgHoverImage(L"Resources/minimize_h.png");
	//minimizeBtn->SetSizeFromBgImage();
	//minimizeBtn->SetRect(100, 100, 50, 50);

	//auto collapse = mainLayer->AddCollapsable();
	//collapse->SetName("Collapse");
	//
	//collapse->SetCaptionText(L"Collapsable Control");
	//auto btn0 = collapse->AddToList<GuiButton>();
	//auto btn1 = collapse->AddToList<GuiButton>();
	//btn0->SetText(L"Short");
	//btn1->SetText(L"Long long long long");
	//
	//auto collapse2 = collapse->Clone();
	//collapse2->SetName(L"EZAZ");
	//collapse->AddToList(collapse2);
	//
	////btn0->SetBorder(1, Color::WHITE);
	////btn1->SetBorder(1, Color::BLUE);
	//collapse2->SetBorder(1, Color::BLUE);
	//collapse->SetBorder(1, Color::RED);
	////collapse->AddToList(collapse->Clone());
	//
	//collapse->SetRect(200, 200, 200, 200);
	return;





	//auto button = mainLayer->AddButton();
	//button->SetRect(0, 0, 100, 100);
	//button->SetText("halleluja");
	//button->SetBgToColor(Color(30), Color(100));
	// Control List
	//eTextAlign align = eTextAlign::CENTER;
	//GuiList* list = mainLayer->AddList();
	//GuiButton* button = list->AddButton();
	//button->SetText("File");
	//button->SetTextAlign(align);
	//GuiButton* button1 = list->AddButton();
	//button1->SetText("Edit");
	//button1->SetTextAlign(align);
	//GuiButton* button2 = list->AddButton();
	//button2->SetText("Project");
	//button2->SetTextAlign(align);
	//GuiButton* button3 = list->AddButton();
	//button3->SetText("Resources");
	//button3->SetTextAlign(align);
	//GuiButton* button4 = list->AddButton();
	//button4->SetText("Help");
	//button4->SetTextAlign(align);
	//list->SetDirection(eGuiListDirection::HORIZONTAL);

	//GuiList* menu = mainLayer->AddList();
	//GuiButton* fileBtn = menu->AddButton();
	//fileBtn->SetText("File");
	//GuiButton* editBtn = menu->AddButton();
	//editBtn->SetText("Edit");
	//GuiButton* projectBtn = menu->AddButton();
	//projectBtn->SetText("Projects");
	//menu->SetRect(0, 0, 200, 30);



	//GuiButton* button = mainLayer->AddButton();
	//GuiButton* button2 = mainLayer->AddButton();
	//GuiButton* button3 = mainLayer->AddButton();

	// Menu
	{
		// Control List
		eTextAlign align = eTextAlign::CENTER;
		GuiList* list = mainLayer->AddList();
		list->SetBorder(1, Color::RED);
		GuiButton* button = list->AddButton();
		button->SetText("File");
		//button->SetTextAlign(align);
		button->SetPadding(4, 4, 4, 4);
		GuiButton* button1 = list->AddButton();
		button1->SetText("Edit");
		//button1->SetTextAlign(align);
		button1->SetPadding(4, 4, 4, 4);
		GuiButton* button2 = list->AddButton();
		button2->SetText("Project");
		//button2->SetTextAlign(align);
		button2->SetPadding(4, 4, 4, 4);
		GuiButton* button3 = list->AddButton();
		button3->SetText("Resources");
		//button3->SetTextAlign(align);
		button3->SetPadding(4, 4, 4, 4);
		GuiButton* button4 = list->AddButton();
		button4->SetText("Help");
		//button4->SetTextAlign(align);
		button4->SetPadding(4, 4, 4, 4);
		list->SetDirection(eGuiListDirection::HORIZONTAL);

		list->SetBgColorForAllStates(Color(0));
		list->SetRect(0, 0, 0, 0);
		//{
		//	auto button = mainLayer->AddButton();
		//	button->SetBgToColor(Color(55, 55, 55), Color(80, 80, 80));
		//	button->SetRect(0, 0, 60, 22);
		//	button->SetText("File");
		//}
		//
		////button->onClick += [](CursorEvent& evt) {MessageBoxA(NULL, "Click", "Click", MB_OK); };
		////button->onPress += [](CursorEvent& evt) {MessageBoxA(NULL, "Press", "Press", MB_OK); };
		////button->onRelease += [](CursorEvent& evt) {MessageBoxA(NULL, "Release", "Release", MB_OK); };
		////button->onMouseEnter += [](CursorEvent& evt) {MessageBoxA(NULL, "Enter", "Enter", MB_OK); };
		////button->onMouseLeave += [](CursorEvent& evt) {MessageBoxA(NULL, "Leave", "Leave", MB_OK); };
		////button->onMouseHover += [](CursorEvent& evt) {MessageBoxA(NULL, "Hover", "Hover", MB_OK); };
		//
		//button2 = mainLayer->AddButton();
		//button2->SetBackgroundToColor(Color(55, 55, 55), Color(80, 80, 80));
		//button2->SetRect(61, 0, 60, 22);
		//button2->SetText("Edit");
		//
		//button3 = mainLayer->AddButton();
		//button3->SetBackgroundToColor(Color(55, 55, 55), Color(80, 80, 80));
		//button3->SetRect(122, 0, 60, 22);
		//button3->SetText("About");
		//
		//button3 = mainLayer->AddButton();
		//button3->SetBackgroundToColor(Color(55, 55, 55), Color(80, 80, 80));
		//button3->SetRect(122, 0, 60, 22);
		//button3->SetText("About");
	}
	//
	//
	//// Node1
	//{
	//int x = 400;
	//int y = 100;
	//Vector2f pinSize = { 10, 10 };
	//float pinSpace = 20.f;
	//button = mainLayer->AddButton();
	//button->SetBgToColor(Color(55), Color(80));
	//button->SetName("NODE");
	//button->SetRect(x, y, 60, 60);
	//button->SetText("Node1");
	//button->onMouseClick += [](Gui* self, CursorEvent& evt) {MessageBoxA(NULL, "Node1Click", "Node1Click", MB_OK); };
	//GuiButton* pin0 = button->AddButton();
	//pin0->SetName("PIN");
	//pin0->SetBgToColor(Color(255), Color(255));
	//pin0->SetRect(x - pinSize.x * 0.5, y + pinSize.y * 0.5, pinSize.x, pinSize.y);
	//pin0->onMouseClick += [](Gui* self, CursorEvent& evt) {MessageBoxA(NULL, "pin0Click", "pin0Click", MB_OK); };
	//GuiButton* pin1 = button->AddButton();
	//pin1->SetBgToColor(Color(100), Color(150));
	//pin1->SetRect(x - pinSize.x * 0.5, y + pinSize.y * 0.5 + pinSpace, pinSize.x, pinSize.y);
	//pin1->onMouseClick += [](Gui* self, CursorEvent& evt) {MessageBoxA(NULL, "pin1Click", "pin1Click", MB_OK); };
	//GuiButton* pin2 = button->AddButton();
	//pin2->SetBgToColor(Color(100), Color(150));
	//pin2->SetRect(x - pinSize.x * 0.5, y + pinSize.y * 0.5 + pinSpace * 2.f, pinSize.x, pinSize.y);
	//pin2->onMouseClick += [](Gui* self, CursorEvent& evt) {MessageBoxA(NULL, "pin2Click", "pin2Click", MB_OK); };
	//GuiButton* outputPin = button->AddButton();
	//outputPin->SetBgToColor(Color(100), Color(150));
	//outputPin->SetRect(x + 60 - pinSize.x * 0.5, y + 30 - pinSize.y * 0.5, pinSize.x, pinSize.y);
	//outputPin->onMouseClick += [](Gui* self, CursorEvent& evt) {MessageBoxA(NULL, "outputPin", "outputPin", MB_OK); };
	//}

	Gui* node2;
	//Node2
	{
		int x = 300;
		int y = 100;
		Vector2f pinSize = { 10, 10 };
		float pinSpace = 20.f;
		GuiButton* button = mainLayer->AddButton();
		button->DisableClipChildren();

		//button->DisableClip();
		button->SetBgToColor(Color(45), Color(50));
		button->SetRect(x, y, 60, 60);
		button->SetText("Node2");
		button->AlignCenter();
		GuiButton* pin0 = button->AddButton();
		node2 = pin0;
		//pin0->DisableClip();
		pin0->SetBgToColor(Color(120), Color(180));
		pin0->SetRect(x - pinSize.x() * 0.5, y + pinSize.y() * 0.5, pinSize.x(), pinSize.y());
		GuiButton* pin1 = button->AddButton();
		//pin1->DisableClip();
		pin1->SetBgToColor(Color(120), Color(180));
		pin1->SetRect(x - pinSize.x() * 0.5, y + pinSize.y() * 0.5 + pinSpace, pinSize.x(), pinSize.y());
		GuiButton* pin2 = button->AddButton();
		pin2->SetBgToColor(Color(120), Color(180));
		pin2->SetRect(x - pinSize.x() * 0.5, y + pinSize.y() * 0.5 + pinSpace * 2.f, pinSize.x(), pinSize.y());
		//pin2->DisableClip();
		GuiButton* outputPin = button->AddButton();
		outputPin->SetBgToColor(Color(120), Color(180));
		outputPin->SetRect(x + 60 - pinSize.x() * 0.5, y + 30 - pinSize.y() * 0.5, pinSize.x(), pinSize.y());
		//outputPin->DisableClip();
	}


	{
		// Control List
		eTextAlign align = eTextAlign::LEFT;

		GuiList* list = mainLayer->AddList();
		list->SetBorder(1, Color(0, 0, 255));
		list->SetDirection(eGuiListDirection::HORIZONTAL);

		GuiButton* button = list->AddButton();
		button->SetBorder(1, Color(255, 0, 0));
		button->SetText("Button");
		//button->SetTextAlign(align);

		GuiButton* button1 = list->AddButton();
		button1->SetText("Text");
		//button1->SetTextAlign(align);
		button1->SetBorder(1, Color(0, 255, 0));

		list->SetRect(0, 70, 60, 600);

		//GuiList* clone = list->Clone();
		//list->Add(clone);
		//list->SetRect(50, 200, 60, 600);
		int asd = 5;
		asd++;
	}

	{
		// Control List
		eTextAlign align = eTextAlign::LEFT;
		GuiList* list = mainLayer->AddList();
		GuiButton* button = list->AddButton();
		button->SetText("Disconnect");
		//button->SetTextAlign(align);
		//button->SetRect(0, 0, 10, 20);
		GuiButton* button1 = list->AddButton();
		button1->SetText(L"Amit csak ákársz vaze");
		//button1->SetTextAlign(align);
		//button1->SetRect(0, 0, 10, 20);
		//list->SetStride(25);
		list->SetRect(0, 70, 150, 600);
		GuiList* clone = list->Clone();
		clone->Remove(); // TODO clone policy, without parenting
		node2->SetContextMenu(clone);
		list->Remove();
	}

	// Slider o yeah
	{
		GuiSlider* slider = mainLayer->AddSlider();
		slider->SetRect(200, 500, 100, 15);

		GuiSlider* slider2 = mainLayer->AddSlider();
		slider2->SetValue(0.7);
		slider2->SetRect(200, 520, 100, 15);

		GuiSlider* slider3 = mainLayer->AddSlider();
		slider3->SetValue(0.5);
		slider3->SetRect(200, 540, 100, 15);
	}

	////Image test
	//{
	//	Gui* p = mainLayer->AddPlane();
	//	//p->SetImageForAllStates(L"c:\\UE4Interface_5.jpg");
	//	p->SetRect(0, 0, 800, 200);
	//	Gui* clone = p->Clone();
	//	clone->SetRect(0, 200, 800, 200);
	//	//p->Clone()->SetRect(0, 400, 800, 200);
	//	//p->Clone()->SetRect(0, 600, 800, 200);
	//	//p->Clone()->SetRect(0, 800, 800, 200);
	//}
}

void Editor::Run()
{
	// Create timer, delta time -> engine
	Timer* timer = new Timer();
	timer->Start();

	wnd->SetTitle(L"Inline Editor");

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

		//Sleep(200);
	}
	
	delete timer;
}

LRESULT Editor::WndProc(HWND handle, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRet;
	bool fCallDWP = !DwmDefWindowProc(handle, msg, wParam, lParam, &lRet);

	switch (msg)
	{
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
				maximizeBtn->InitFromImage(L"Resources/restore.png", L"Resources/restore_h.png");
				bWndMaximized = true;
			}
			else if (wParam == SIZE_RESTORED)
			{
				maximizeBtn->InitFromImage(L"Resources/maximize.png", L"Resources/maximize_h.png");
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

		//pncsp->rgrc[0].left = pncsp->rgrc[0].left + 0;
		//pncsp->rgrc[0].top = pncsp->rgrc[0].top + 0;
		//pncsp->rgrc[0].right = pncsp->rgrc[0].right - 0;
		//pncsp->rgrc[0].bottom = pncsp->rgrc[0].bottom - 0;

		lRet = 0;

		// No need to pass the message on to the DefWindowProc.
		fCallDWP = false;

		break;
	}
	case WM_NCHITTEST:
	{	
		Vector2i cursorPos = guiE->GetCursorPos();
		
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
		else if (closeBtn->IsCursorInside())
		{
			// HTNOWHERE
		}
		else if (maximizeBtn->IsCursorInside())
		{
			// HTNOWHERE
		}
		else if (minimizeBtn->IsCursorInside())
		{
			// HTNOWHERE
		}
		else if (captionBar->IsCursorInside())
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
}