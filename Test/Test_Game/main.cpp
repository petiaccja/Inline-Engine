#include "DebugInfoFrame.hpp"
#include "GameInterface.hpp"
#include "GameWorld.hpp"
#include "MainMenuFrame.hpp"
#include "UserInterfaceCompositor.hpp"
#include "UserInterfaceSystem.hpp"
#include "WindowEventSystem.hpp"

#include <BaseLibrary/Platform/Window.hpp>
#include <BaseLibrary/Timer.hpp>

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

		compositor.GetBinding(debugInfoFrame).SetAnchors(true, false, false, true);
		debugInfoFrame.SetSize({ 300, 150 });
		debugInfoFrame.SetAdapterInfo(modules.GetGraphicsAdapter());

		compositor.GetBinding(mainMenuFrame).SetAnchors(true, true, true, true).SetResizing(false);
		mainMenuFrame.SetSize({ 200, 350 });
		mainMenuFrame.OnQuit += [&window] { window.Close(); };

		auto& weSystem = gameWorld.GetWorld().GetSystem<WindowEventSystem>();
		auto& uiSystem = gameWorld.GetWorld().GetSystem<UserInterfaceSystem>();
		weSystem.SetWindows({ &window });
		uiSystem.SetBoards({ &gameInterface.GetBoard() });

		SetEvents(window, gameWorld, gameInterface, modules.GetGraphicsEngine(), compositor);

		window.SetSize(Vec2u{ 640, 480 } + window.GetSize() - window.GetClientSize());

		timer.Start();
		while (!window.IsClosed()) {
			float frameTime = float(timer.Elapsed());
			timer.Reset();
			gameWorld.GetWorld().Update(frameTime);
		}
	}
	catch (std::exception& ex) {
		std::cout << ex.what() << std::endl;
	}
	return 0;
}
