#include "DebugInfoFrame.hpp"
#include "GameInterface.hpp"
#include "GameWorld.hpp"
#include "MainMenuFrame.hpp"
#include "UserInterfaceCompositor.hpp"
#include "UserInterfaceSystem.hpp"
#include "WindowEventSystem.hpp"

#include <BaseLibrary/Platform/Window.hpp>
#include <BaseLibrary/Timer.hpp>
#include "GameSceneFrame.hpp"

#undef interface // fucking shitty winapi as usual


using namespace inl;


void SetEvents(Window& window, GameWorld& world, GameInterface& interface, gxeng::IGraphicsEngine& engine, UserInterfaceCompositor& compositor) {
	window.OnResize += [&](ResizeEvent evt) -> void {
		if (evt.resizeMode == eResizeMode::MINIMIZED) {
			return;
		}
		interface.GetCamera().SetPosition(Vec2(evt.clientSize) / 2.0f);
		interface.GetCamera().SetExtent(Vec2(evt.clientSize));
		interface.GetBoard().SetCoordinateMapping({ 0.f, (float)evt.clientSize.x, (float)evt.clientSize.y, 0.f }, { 0.f, (float)evt.clientSize.x, 0.f, (float)evt.clientSize.y });
		engine.SetScreenSize(evt.clientSize.x, evt.clientSize.y);
		compositor.SetSize(evt.clientSize);
		compositor.SetPosition((Vec2)evt.clientSize / 2.0f);
		try {
			compositor.GetFrame<DebugInfoFrame>().SetResolutionInfo(evt.clientSize);
		}
		catch (...) {
		}
	};

	window.OnKeyboard += Delegate<void(KeyboardEvent)>{ &gui::Board::OnKeyboard, &interface.GetBoard() };
	window.OnMouseButton += Delegate<void(MouseButtonEvent)>{ &gui::Board::OnMouseButton, &interface.GetBoard() };
	window.OnMouseMove += Delegate<void(MouseMoveEvent)>{ &gui::Board::OnMouseMove, &interface.GetBoard() };
	window.OnMouseWheel += Delegate<void(MouseWheelEvent)>{ &gui::Board::OnMouseWheel, &interface.GetBoard() };
}


int main() {
	try {
		Timer timer;
		Window window{ "Test game" };
		ModuleCollection modules{ window.GetNativeHandle() };
		AssetCacheCollection assetCaches{ modules.GetGraphicsEngine() };

		GameWorld gameWorld{ modules, assetCaches };
		GameInterface gameInterface{ modules };
		UserInterfaceCompositor compositor{ gameInterface.GetBoard() };

		DebugInfoFrame& debugInfoFrame = compositor.ShowFrame<DebugInfoFrame>();
		MainMenuFrame& mainMenuFrame = compositor.ShowFrame<MainMenuFrame>();
		GameSceneFrame& gameSceneFrame = compositor.ShowFrame<GameSceneFrame>();

		compositor.GetBinding(debugInfoFrame).SetAnchors(true, false, false, true);
		debugInfoFrame.SetSize({ 300, 150 });
		debugInfoFrame.SetAdapterInfo(modules.GetGraphicsAdapter());

		compositor.GetBinding(mainMenuFrame).SetAnchors(true, true, true, true).SetResizing(false);
		mainMenuFrame.SetSize({ 200, 350 });
		mainMenuFrame.OnQuit += [&window] { window.Close(); };

		gameSceneFrame.SetGameWorld(gameWorld);
		compositor.HideFrame<GameSceneFrame>();

		auto& weSystem = gameWorld.GetSimulation().Get<WindowEventSystem>();
		auto& uiSystem = gameWorld.GetSimulation().Get<UserInterfaceSystem>();
		weSystem.SetWindows({ &window });
		uiSystem.SetBoards({ &gameInterface.GetBoard() });

		gameWorld.GetCamera().SetTarget({ 0, 0, 0 });
		gameWorld.GetCamera().SetPosition({ 5, 5, 2 });
		gameWorld.GetCamera().SetUpVector({ 0, 0, 1 });
		gameWorld.GetCamera().SetNearPlane(0.5f);
		gameWorld.GetCamera().SetFarPlane(200.f);
		gameWorld.GetCamera().SetFOVAspect(70.f, 4.f / 3.f);

		SetEvents(window, gameWorld, gameInterface, modules.GetGraphicsEngine(), compositor);

		window.SetSize(Vec2u{ 640, 480 } + window.GetSize() - window.GetClientSize());

		timer.Start();
		while (!window.IsClosed()) {
			float frameTime = float(timer.Elapsed());
			timer.Reset();
			gameWorld.GetSimulation().Run(gameWorld.GetScene(), frameTime);
		}
	}
	catch (std::exception& ex) {
		std::cout << ex.what() << std::endl;
	}
	return 0;
}
