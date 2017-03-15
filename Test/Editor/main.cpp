// Excessive Engine Editor
// CryEngine, UE4, Unity, Godot, etc.. get REKT

#include <BaseLibrary/Platform/Window.hpp>
#include <BaseLibrary/Platform/Timer.hpp>
#include <GraphicsEngine/IGraphicsEngine.hpp>
#include <GuiEngine/GuiEngine.hpp>
#include <Core/Core.hpp>
#include <Core/InputCore.h>
#include <string>

using namespace inl::gxeng;

Window* window;
GuiEngine* guiEngine;
IGraphicsEngine* graphicsEngine;

void InitGameScripts();
void InitGui();

void main()
{
	// Create Game Window
	WindowDesc d;
	d.clientSize = uvec2(800, 600);
	d.style = eWindowStyle::BORDERLESS;
	window = new Window(d);

	// Init Graphics Engine
	//GraphicsEngineRasterZsirosDesc graphicsDesc;
	//graphicsDesc.gapiType = eGapiType::DX11;
	//graphicsDesc.targetWindow = window;
	//graphicsEngine = Core.InitGraphicsEngineRasterZsiros(graphicsDesc);
	//graphicsEngine = new
	graphicsEngine = Core.InitGraphicsEngine();

	// Init Gui Engine
	guiEngine = Core.InitGuiEngine(graphicsEngine);

	// Init gui
	InitGui();

	// Init Game Scripts
	InitGameScripts(); // Manual bullshit, TODO !!!

					   // Create timer, delta time -> engine
	Timer* timer = new Timer();
	timer->Start();


	// Main loop, till window open
	while (window->IsOpen())
	{
		// Prepare for input processing
		Input.ClearFrameData();

		// Process input events coming from O.S.-> Window
		WindowEvent evt;
		while (window->PopEvent(evt))
		{
			switch (evt.msg)
			{
			case KEY_PRESS:
			{
				if (evt.key != INVALID_eKey)
					Input.KeyPress(evt.key);
			} break;

			case KEY_RELEASE:
			{
				if (evt.key != INVALID_eKey)
					Input.KeyRelease(evt.key);
			} break;

			case MOUSE_MOVE:
			{
				Input.MouseMove(ivec2(evt.deltaX, evt.deltaY), ivec2(evt.x, evt.y));
			} break;

			case MOUSE_PRESS:
			{
				switch (evt.mouseBtn)
				{
				case LEFT:	Input.MouseLeftPress();		break;
				case RIGHT:	Input.MouseRightPress();	break;
				case MID:	Input.MouseMidPress();		break;
				}
			} break;

			case MOUSE_RELEASE:
			{
				switch (evt.mouseBtn)
				{
				case LEFT:	Input.MouseLeftRelease();	break;
				case RIGHT: Input.MouseRightRelease();	break;
				case MID:	Input.MouseMidRelease();	break;
				}
			} break;
			}
		}

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
		float deltaSeconds = timer->GetSecondsPassed();
		timer->Reset();

		// Go Excessive Engine go !!
		Core.Update(deltaSeconds);
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

	// Test button
	GuiControl* button = layer->AddControl();
	button->SetBackgroundToColor(Color::RED);
	button->SetRect(0, 0, 100, 50);
	//button->SetText("Button");
}