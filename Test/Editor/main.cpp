// Excessive Engine Editor
// CryEngine, UE4, Unity, Godot, etc.. get REKT

#include <BaseLibrary/Platform/Window.hpp>
#include <BaseLibrary/Timer.hpp>
#include <GuiEngine/GuiEngine.hpp>
#include <Core/Core.hpp>
#include <Core/InputCore.h>
#include <string>

using namespace exc;
using namespace inl::gxeng;

Window* window;
GuiEngine* guiEngine;
GraphicsEngine* graphicsEngine;
EngineCore Core;
InputCore Input;

//GuiButton* button;
//GuiButton* button2;
//GuiButton* button3;

void InitGui();
//void InitContextMenuTest();
//void Update(float deltaTime);


void main()
{
	// Create Game Window
	WindowDesc d;
	d.clientSize = Vector2u(800, 600);
	d.style = eWindowStyle::DEFAULT;
	window = new Window(d);

	d.clientSize = Vector2u(800, 600);
	//d.style = eWindowStyle::BORDERLESS;
	auto window2 = new Window(d);

	HWND a = (HWND)window->GetHandle();
	HWND b = (HWND)window2->GetHandle();

	//SetParent(b, a); //a will be the new parent b
	//DWORD style = GetWindowLong(b, GWL_STYLE); //get the b style
	//style &= ~(WS_POPUP | WS_CAPTION); //reset the "caption" and "popup" bits
	//style |= WS_CHILD; //set the "child" bit
	//SetWindowLong(b, GWL_STYLE, style); //set the new style of b
	//RECT rc; //temporary rectangle
	////GetClientRect(a, &rc); //the "inside border" rectangle for a
	////MoveWindow(b, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, true); //place b at (x,y,w,h) in a
	//UpdateWindow(a);

	//std::swap(window, window2);

	// Init Graphics Engine
	graphicsEngine = Core.InitGraphicsEngine(window2->GetClientWidth(), window2->GetClientHeight(), (HWND)window2->GetHandle());

	// Init Gui Engine
	guiEngine = Core.InitGuiEngine(graphicsEngine, window);

	// Init gui
	InitGui();

	// Create timer, delta time -> engine
	Timer* timer = new Timer();
	timer->Start();

	window->SetTitle(L"Inline Gui Editor");

	// Main loop, till window open
	while (window->IsOpen())
	{
		// Prepare for input processing
		Input.ClearFrameData();

		WindowEvent evt;
		while (window->PopEvent(evt));
		while (window2->PopEvent(evt));

		// Process input events coming from O.S.-> Window
		//WindowEvent evt;
		//while (window->PopEvent(evt))
		//{
		//	switch (evt.msg)
		//	{
		//	case KEY_PRESS:
		//	{
		//		if (evt.key != INVALID_eKey)
		//			Input.KeyPress(evt.key);
		//	} break;
		//
		//	case KEY_RELEASE:
		//	{
		//		if (evt.key != INVALID_eKey)
		//			Input.KeyRelease(evt.key);
		//	} break;
		//
		//	case MOUSE_MOVE:
		//	{
		//		Input.MouseMove(Vector2i(evt.deltaX, evt.deltaY), Vector2i(evt.x, evt.y));
		//	} break;
		//
		//	case MOUSE_PRESS:
		//	{
		//		switch (evt.mouseBtn)
		//		{
		//		case LEFT:	Input.MouseLeftPress();		break;
		//		case RIGHT:	Input.MouseRightPress();	break;
		//		case MID:	Input.MouseMidPress();		break;
		//		}
		//	} break;
		//
		//	case MOUSE_RELEASE:
		//	{
		//		switch (evt.mouseBtn)
		//		{
		//		case LEFT:	Input.MouseLeftRelease();	break;
		//		case RIGHT: Input.MouseRightRelease();	break;
		//		case MID:	Input.MouseMidRelease();	break;
		//		}
		//	} break;
		//	}
		//}

		// Dispatch Inputs
		Input.Update();

		// IsKeyPressed Enterre sose lesz igaz, mert asszem hogy a window message - ben kétszer szerepel az enter megnyomása, mert system message is jön, meg egy nem system message is
		//if (Input.IsKeyPressed(ENTER))
		//{
		//	uint32_t width = window->GetClientWidth();
		//	uint32_t height = window->GetClientHeight();
		//
		//	graphicsEngine->SetResolution(width, height);
		//}

		// Get delta seconds from the timer
		float deltaTime = timer->Elapsed();
		timer->Reset();

		// Go Excessive Engine go !!
		Core.Update(deltaTime);

		Sleep(30);
	}

	// Cleanup
	delete window;
	delete timer;
}

void InitGui()
{
	// New Layer
	GuiLayer* layer = guiEngine->AddLayer();
	//GuiText* btn = layer->AddText();
	//btn->SetText(L"Button");
	//btn->SetPadding(10);
	//btn->SetBgColorForAllStates(Color(50));
	//btn->SetPos(50, 50);
	//btn->SetAlign(eTextAlign::CENTER);
	//btn->SetBorder(1, Color::RED);

	// GuiButton létrejötténél nem tudott a padding, ezért nem helyezõdik el jól a GuiText a buttonban... a GetClientRect() padding nélkül...
	// Megoldás hogy GuiButton transform változásnál szépen rearrangolja a child controlokat alignment policy - k és egyéb policy - k alapján !!!
	GuiButton* btn2 = layer->AddButton();
	btn2->SetRect(0, 0, 100, 200);
	btn2->SetText(L"Button");
	btn2->SetBgColorForAllStates(Color(50));
	btn2->SetTextAlign(eTextAlign::CENTER);
	btn2->SetPadding(5);
	btn2->SetBorder(3, Color::RED);

	return;
	// Gui collapsable
	{
		GuiCollapsable* collapsable = layer->AddCollapsable();
		collapsable->SetName("collapsable");
		collapsable->SetCaptionText(L"Collapsable");
		collapsable->SetRect(500, 500, 60, 30);
		collapsable->AddToList<GuiButton>()->SetText("LOL");
		collapsable->AddToList<GuiButton>()->SetText("LOL2");
		//collapsable->SetMargin(10, 10, 10, 10);
		collapsable->SetBgColorForAllStates(Color(150));

		GuiCollapsable* collapsable2 = layer->AddCollapsable();
		collapsable2->SetName("Collapsable2");
		collapsable2->SetCaptionText(L"Collapsable2");
		collapsable2->SetRect(500, 520, 60, 30);
		collapsable2->AddToList<GuiButton>()->SetText("LOL");
		collapsable2->AddToList<GuiButton>()->SetText("LOL2");
		collapsable2->SetBgColorForAllStates(Color(0, 0, 0, 0));
		
		GuiCollapsable* collapsableList = layer->AddCollapsable();
		collapsableList->SetCaptionText(L"CaptionText");
		collapsableList->SetName("CollapsableList");
		collapsableList->AddToList(collapsable);
		collapsableList->AddToList(collapsable2);

		collapsableList->SetFitToChildren(true);
		collapsableList->SetRect(500, 400, 10, 250);
		collapsableList->SetBorder(1, Color::WHITE);
		collapsableList->SetBgColorForAllStates(Color(150));

		Widget* caption = collapsableList->GetCaption();
		caption->SetBgIdleColor(Color(20));
		caption->SetBgHoverColor(Color(0));

		int asd = 5;
		asd++;
	}
	//return;
	//auto button = layer->AddButton();
	//button->SetRect(0, 0, 100, 100);
	//button->SetText("halleluja");
	//button->SetBgToColor(Color(30), Color(100));
	// Control List
	//eTextAlign align = eTextAlign::CENTER;
	//GuiList* list = layer->AddList();
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

	//GuiList* menu = layer->AddList();
	//GuiButton* fileBtn = menu->AddButton();
	//fileBtn->SetText("File");
	//GuiButton* editBtn = menu->AddButton();
	//editBtn->SetText("Edit");
	//GuiButton* projectBtn = menu->AddButton();
	//projectBtn->SetText("Projects");
	//menu->SetRect(0, 0, 200, 30);



	//GuiButton* button = layer->AddButton();
	//GuiButton* button2 = layer->AddButton();
	//GuiButton* button3 = layer->AddButton();
	
	// Menu
	{
		// Control List
		eTextAlign align = eTextAlign::CENTER;
		GuiList* list = layer->AddList();
		list->SetBorder(1, Color::RED);
		GuiButton* button = list->AddButton();
		button->SetText("File");
		button->SetTextAlign(align);
		button->SetPadding(4, 4, 4, 4);
		GuiButton* button1 = list->AddButton();
		button1->SetText("Edit");
		button1->SetTextAlign(align);
		button1->SetPadding(4, 4, 4, 4);
		GuiButton* button2 = list->AddButton();
		button2->SetText("Project");
		button2->SetTextAlign(align);
		button2->SetPadding(4, 4, 4, 4);
		GuiButton* button3 = list->AddButton();
		button3->SetText("Resources");
		button3->SetTextAlign(align);
		button3->SetPadding(4, 4, 4, 4);
		GuiButton* button4 = list->AddButton();
		button4->SetText("Help");
		button4->SetTextAlign(align);
		button4->SetPadding(4, 4, 4, 4);
		list->SetDirection(eGuiListDirection::HORIZONTAL);
	
		list->SetBgColorForAllStates(Color(0));
		list->SetRect(0, 0, 0, 0);
		//{
		//	auto button = layer->AddButton();
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
	//button2 = layer->AddButton();
	//button2->SetBackgroundToColor(Color(55, 55, 55), Color(80, 80, 80));
	//button2->SetRect(61, 0, 60, 22);
	//button2->SetText("Edit");
	//
	//button3 = layer->AddButton();
	//button3->SetBackgroundToColor(Color(55, 55, 55), Color(80, 80, 80));
	//button3->SetRect(122, 0, 60, 22);
	//button3->SetText("About");
	//
	//button3 = layer->AddButton();
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
	//button = layer->AddButton();
	//button->SetBgToColor(Color(55), Color(80));
	//button->SetName("NODE");
	//button->SetRect(x, y, 60, 60);
	//button->SetText("Node1");
	//button->onMouseClick += [](Widget* self, CursorEvent& evt) {MessageBoxA(NULL, "Node1Click", "Node1Click", MB_OK); };
	//GuiButton* pin0 = button->AddButton();
	//pin0->SetName("PIN");
	//pin0->SetBgToColor(Color(255), Color(255));
	//pin0->SetRect(x - pinSize.x * 0.5, y + pinSize.y * 0.5, pinSize.x, pinSize.y);
	//pin0->onMouseClick += [](Widget* self, CursorEvent& evt) {MessageBoxA(NULL, "pin0Click", "pin0Click", MB_OK); };
	//GuiButton* pin1 = button->AddButton();
	//pin1->SetBgToColor(Color(100), Color(150));
	//pin1->SetRect(x - pinSize.x * 0.5, y + pinSize.y * 0.5 + pinSpace, pinSize.x, pinSize.y);
	//pin1->onMouseClick += [](Widget* self, CursorEvent& evt) {MessageBoxA(NULL, "pin1Click", "pin1Click", MB_OK); };
	//GuiButton* pin2 = button->AddButton();
	//pin2->SetBgToColor(Color(100), Color(150));
	//pin2->SetRect(x - pinSize.x * 0.5, y + pinSize.y * 0.5 + pinSpace * 2.f, pinSize.x, pinSize.y);
	//pin2->onMouseClick += [](Widget* self, CursorEvent& evt) {MessageBoxA(NULL, "pin2Click", "pin2Click", MB_OK); };
	//GuiButton* outputPin = button->AddButton();
	//outputPin->SetBgToColor(Color(100), Color(150));
	//outputPin->SetRect(x + 60 - pinSize.x * 0.5, y + 30 - pinSize.y * 0.5, pinSize.x, pinSize.y);
	//outputPin->onMouseClick += [](Widget* self, CursorEvent& evt) {MessageBoxA(NULL, "outputPin", "outputPin", MB_OK); };
	//}
	
	Widget* node2;
	//Node2
	{
	int x = 300;
	int y = 100;
	Vector2f pinSize = { 10, 10 };
	float pinSpace = 20.f;
	GuiButton* button = layer->AddButton();
	button->DisableClipChildren();

	//button->DisableClip();
	button->SetBgToColor(Color(45), Color(50));
	button->SetRect(x, y, 60, 60);
	button->SetText("Node2");
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

	GuiList* list = layer->AddList();
	list->SetBorder(1, Color(0, 0, 255));
	list->SetDirection(eGuiListDirection::HORIZONTAL);

	GuiButton* button = list->AddButton();
	button->SetBorder(1, Color(255, 0, 0));
	button->SetText("Button");
	button->SetTextAlign(align);

	GuiButton* button1 = list->AddButton();
	button1->SetText("Text");
	button1->SetTextAlign(align);
	button1->SetBorder(1, Color(0, 255, 0));

	list->SetRect(0, 70, 60, 600);

	GuiList* clone = list->Clone();
	//list->Add(clone);
	//list->SetRect(50, 200, 60, 600);
	int asd = 5;
	asd++;
	}

	{
	// Control List
	eTextAlign align = eTextAlign::LEFT;
	GuiList* list = layer->AddList();
	GuiButton* button = list->AddButton();
	button->SetText("Disconnect");
	button->SetTextAlign(align);
	//button->SetRect(0, 0, 10, 20);
	GuiButton* button1 = list->AddButton();
	button1->SetText(L"Amit csak ákársz vaze");
	button1->SetTextAlign(align);
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
		GuiSlider* slider = layer->AddSlider();
		slider->SetRect(200, 500, 100, 15);
	
		GuiSlider* slider2 = layer->AddSlider();
		slider2->SetValue(0.7);
		slider2->SetRect(200, 520, 100, 15);
	
		GuiSlider* slider3 = layer->AddSlider();
		slider3->SetValue(0.5);
		slider3->SetRect(200, 540, 100, 15);
	}

	////Image test
	//{
	//	Widget* p = layer->AddPlane();
	//	//p->SetImageForAllStates(L"c:\\UE4Interface_5.jpg");
	//	p->SetRect(0, 0, 800, 200);
	//	Widget* clone = p->Clone();
	//	clone->SetRect(0, 200, 800, 200);
	//	//p->Clone()->SetRect(0, 400, 800, 200);
	//	//p->Clone()->SetRect(0, 600, 800, 200);
	//	//p->Clone()->SetRect(0, 800, 800, 200);
	//}
}

//void InitContextMenuTest()
//{
//	// New Layer
//	GuiLayer* layer = guiEngine->AddLayer();
//
//	GuiList* list = layer->AddList();
//	GuiButton* button = list->AddButton();
//	button->SetRect(0, 0, 50, 50);
//	button->SetText("Button");
//	list->SetRect(0, 0, 50, 100);//TODO Enélkül fos a clone
//	GuiList* contextMenu = list->Clone();
//
//	button->background->SetName("ORIG_PLANE");
//	contextMenu->GetChildren()[0]->AsButton()->background->SetName("CLONE_PLANE");
//	//button->background->SetName("PLANE");
//	//contextMenu->SetName("clone");
//
//	button->SetContextMenu(contextMenu);
//
//	// Tehát Widget* self átadása nem hülyeség, mert így a user ha akar akkor leszármazás nélkül is tud klónolni lokális viselkedést !
//	// A usernek legyen lehetõsége arra is hogy csak layout - ot cloneoljon
//	//Clone
//	//CloneWithoutEvents
//}

void Update(float deltaTime)
{
	//button->Move(deltaTime * 5, deltaTime * 5);
	//button2->Move(deltaTime * 5, deltaTime * 5);
	//button3->Move(deltaTime * 5, deltaTime * 5);
}