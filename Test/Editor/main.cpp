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

GuiButton* button;
GuiButton* button2;
GuiButton* button3;

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

	
	window->SetTitle(L"Inline Engine Editor");
	// Main loop, till window open
	while (window->IsOpen())
	{
		// Prepare for input processing
		Input.ClearFrameData();

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
	//GuiCanvas* canvas = guiEngine->AddCanvas();
	//GuiLayer* layer = canvas->AddLayer();

	// Test button
	//button = layer->AddButton();
	//button->SetBackgroundToColor(Color(55, 55, 55));
	//button->SetRect(0, 0, 60, 22);
	//button->SetText("File");
	//
	//button2 = layer->AddButton();
	//button2->SetBackgroundToColor(Color(55, 55, 55));
	//button2->SetRect(61, 0, 60, 22);
	//button2->SetText("Edit");
	//
	//button3 = layer->AddButton();
	//button3->SetBackgroundToColor(Color(55, 55, 55));
	//button3->SetRect(122, 0, 60, 22);
	//button3->SetText("About");
}

void Update(float deltaTime)
{
	button->Move(deltaTime * 5, deltaTime * 5);
	button2->Move(deltaTime * 5, deltaTime * 5);
	button3->Move(deltaTime * 5, deltaTime * 5);
}