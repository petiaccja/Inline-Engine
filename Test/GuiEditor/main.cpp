// Excessive Engine Editor
// CryEngine, UE4, Unity, Godot, etc.. get REKT

#include <BaseLibrary/Platform/Window.hpp>
#include <BaseLibrary/Timer.hpp>
#include <GraphicsEngine/IGraphicsEngine.hpp>
#include <GuiEngine/GuiEngine.hpp>
#include <Core/Core.hpp>
#include <Core/InputCore.h>
#include <string>

using namespace exc;
using namespace inl::gxeng;

Window* window;
GuiEngine* guiEngine;
IGraphicsEngine* graphicsEngine;
EngineCore Core;
InputCore Input;

//GuiButton* button;
//GuiButton* button2;
//GuiButton* button3;

void InitGameScripts();
void InitGui();
void Update(float deltaTime);


void main()
{
	// Create Game Window
	WindowDesc d;
	d.clientSize = uvec2(800, 600);
	d.style = eWindowStyle::DEFAULT;
	window = new Window(d);

	// Init Graphics Engine
	graphicsEngine = Core.InitGraphicsEngine(window->GetClientWidth(), window->GetClientHeight(), (HWND)window->GetHandle());

	// Init Gui Engine
	guiEngine = Core.InitGuiEngine(*graphicsEngine, *window);

	// Init gui
	InitGui();

	// Init Game Scripts
	InitGameScripts(); // Manual bullshit, TODO !!!

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
		//		Input.MouseMove(ivec2(evt.deltaX, evt.deltaY), ivec2(evt.x, evt.y));
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
		if (Input.IsKeyPressed(ENTER))
		{
			uint32_t width = window->GetClientWidth();
			uint32_t height = window->GetClientHeight();

			graphicsEngine->SetResolution(width, height);
		}

		// Get delta seconds from the timer
		float deltaTime = timer->Elapsed();
		timer->Reset();

		Update(deltaTime);

		// Go Excessive Engine go !!
		Core.Update(deltaTime);
		Sleep(30);
	}

	// Cleanup
	delete window;
	delete timer;
}

void InitGameScripts()
{
	//World.AddScript<TestLevelScript>();
}

void InitGui()
{
	// Canvas and Layer
	GuiCanvas* canvas = guiEngine->AddCanvas();
	GuiLayer* layer = canvas->AddLayer();

	GuiButton* button;
	GuiButton* button2;
	GuiButton* button3;

	// Menu
	{
	button = layer->AddButton();
	button->SetBackgroundToColor(Color(55, 55, 55), Color(80, 80, 80));
	button->SetRect(0, 0, 60, 22);
	button->SetText("File");

	button->OnCursorClick += [](CursorEvent& evt) {MessageBoxA(NULL, "Click", "Click", MB_OK); };
	button->OnCursorPress += [](CursorEvent& evt) {MessageBoxA(NULL, "Press", "Press", MB_OK); };
	button->OnCursorRelease += [](CursorEvent& evt) {MessageBoxA(NULL, "Release", "Release", MB_OK); };
	button->OnCursorEnter += [](CursorEvent& evt) {MessageBoxA(NULL, "Enter", "Enter", MB_OK); };
	button->OnCursorLeave += [](CursorEvent& evt) {MessageBoxA(NULL, "Leave", "Leave", MB_OK); };
	button->OnCursorStay += [](CursorEvent& evt) {MessageBoxA(NULL, "Stay", "Stay", MB_OK); };

	button2 = layer->AddButton();
	button2->SetBackgroundToColor(Color(55, 55, 55), Color(80, 80, 80));
	button2->SetRect(61, 0, 60, 22);
	button2->SetText("Edit");

	button3 = layer->AddButton();
	button3->SetBackgroundToColor(Color(55, 55, 55), Color(80, 80, 80));
	button3->SetRect(122, 0, 60, 22);
	button3->SetText("About");
	}


	// Node1
	{
	{
	int x = 400;
	int y = 100;
	vec2 pinSize = { 10, 10 };
	float pinSpace = 20.f;
	button = layer->AddButton();
	button->SetBackgroundToColor(Color(55), Color(80));
	button->SetRect(x, y, 60, 60);
	button->SetText("Node1");
	button->OnCursorClick += [](CursorEvent& evt) {MessageBoxA(NULL, "Node1Click", "Node1Click", MB_OK); };
	GuiButton* pin0 = button->AddButton();
	pin0->SetBackgroundToColor(Color(100), Color(150));
	pin0->SetRect(x - pinSize.x * 0.5, y + pinSize.y * 0.5, pinSize.x, pinSize.y);
	pin0->OnCursorClick += [](CursorEvent& evt) {MessageBoxA(NULL, "pin0Click", "pin0Click", MB_OK); };
	GuiButton* pin1 = button->AddButton();
	pin1->SetBackgroundToColor(Color(100), Color(150));
	pin1->SetRect(x - pinSize.x * 0.5, y + pinSize.y * 0.5 + pinSpace, pinSize.x, pinSize.y);
	pin1->OnCursorClick += [](CursorEvent& evt) {MessageBoxA(NULL, "pin1Click", "pin1Click", MB_OK); };
	GuiButton* pin2 = button->AddButton();
	pin2->SetBackgroundToColor(Color(100), Color(150));
	pin2->SetRect(x - pinSize.x * 0.5, y + pinSize.y * 0.5 + pinSpace * 2.f, pinSize.x, pinSize.y);
	pin2->OnCursorClick += [](CursorEvent& evt) {MessageBoxA(NULL, "pin2Click", "pin2Click", MB_OK); };
	GuiButton* outputPin = button->AddButton();
	outputPin->SetBackgroundToColor(Color(100), Color(150));
	outputPin->SetRect(x + 60 - pinSize.x * 0.5, y + 30 - pinSize.y * 0.5, pinSize.x, pinSize.y);
	outputPin->OnCursorClick += [](CursorEvent& evt) {MessageBoxA(NULL, "outputPin", "outputPin", MB_OK); };
	}

	//Node2
	{
	int x = 300;
	int y = 100;
	vec2 pinSize = { 10, 10 };
	float pinSpace = 20.f;
	button = layer->AddButton();
	button->SetBackgroundToColor(Color(55), Color(80));
	button->SetRect(x, y, 60, 60);
	button->SetText("Node2");
	GuiButton* pin0 = button->AddButton();
	pin0->SetBackgroundToColor(Color(100), Color(150));
	pin0->SetRect(x - pinSize.x * 0.5, y + pinSize.y * 0.5, pinSize.x, pinSize.y);
	GuiButton* pin1 = button->AddButton();
	pin1->SetBackgroundToColor(Color(100), Color(150));
	pin1->SetRect(x - pinSize.x * 0.5, y + pinSize.y * 0.5 + pinSpace, pinSize.x, pinSize.y);
	GuiButton* pin2 = button->AddButton();
	pin2->SetBackgroundToColor(Color(100), Color(150));
	pin2->SetRect(x - pinSize.x * 0.5, y + pinSize.y * 0.5 + pinSpace * 2.f, pinSize.x, pinSize.y);
	GuiButton* outputPin = button->AddButton();
	outputPin->SetBackgroundToColor(Color(100), Color(150));
	outputPin->SetRect(x + 60 - pinSize.x * 0.5, y + 30 - pinSize.y * 0.5, pinSize.x, pinSize.y);
	}
	}

	// Control List
	{
		eTextAlign align = eTextAlign::LEFT;
		GuiList* list = layer->AddList();
		GuiButton* button = list->AddButton();
		button->SetText("Button");
		button->SetTextAlign(align);
		GuiButton* button1 = list->AddButton();
		button1->SetText("Text");
		button1->SetTextAlign(align);
		GuiButton* button2 = list->AddButton();
		button2->SetText("Slider");
		button2->SetTextAlign(align);
		GuiButton* button3 = list->AddButton();
		button3->SetText("List");
		button3->SetTextAlign(align);
		GuiButton* button4 = list->AddButton();
		button4->SetText("MenuBar");
		button4->SetTextAlign(align);
		GuiButton* button5 = list->AddButton();
		button5->SetText("Menu");
		button5->SetTextAlign(align);
		GuiButton* button6 = list->AddButton();
		button6->SetText("Splitter");
		button6->SetTextAlign(align);
		list->SetStride(25);
		list->SetRect(0, 35, 60, 600);
	}
}

void Update(float deltaTime)
{
	//button->Move(deltaTime * 5, deltaTime * 5);
	//button2->Move(deltaTime * 5, deltaTime * 5);
	//button3->Move(deltaTime * 5, deltaTime * 5);
}