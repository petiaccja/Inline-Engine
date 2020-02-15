#include "Game.hpp"

#include <BaseLibrary/Platform/Window.hpp>
#include <BaseLibrary/Timer.hpp>

#undef interface // fucking shitty winapi as usual


using namespace inl;


void SetEvents(Window& window, Game& game, gxeng::IGraphicsEngine& engine) {
	window.OnResize += [&](ResizeEvent evt) -> void {
		if (evt.resizeMode == eResizeMode::MINIMIZED) {
			return;
		}
		engine.SetScreenSize(evt.clientSize.x, evt.clientSize.y);
	};
}


int main() {
	try {
		Timer timer;
		Window window{ "Test game" };
		EngineCollection modules{ window.GetNativeHandle() };
		Game gameWorld{ modules, window };

		SetEvents(window, gameWorld, modules.GetGraphicsEngine());

		window.SetSize(Vec2u{ 640, 480 } + window.GetSize() - window.GetClientSize());

		timer.Start();
		while (!window.IsClosed()) {
			// Compute frametime.
			float frameTime = float(timer.Elapsed());
			timer.Reset();

			gameWorld.Update(frameTime);
		}
	}
	catch (Exception& ex) {
		std::cout << "\033[91mUnhandled exception:\033[m\n" << std::endl;
		std::cout << "\033[33mStack trace:\033[m" << std::endl;
		ex.PrintStackTrace(std::cout);
		std::cout << "\033[33mStack trace ^^\033[m" << std::endl;
		std::cout << std::endl;
		std::cout << "\033[33m" << ex.Message() << "\033[m" << std::endl;
		std::cout << "\033[95m  >> " << ex.Subject() << "\033[m" << std::endl;
	}
	catch (std::exception& ex) {
		std::cout << "\033[91mUnhandled exception:\033[m\n" << std::endl;
		std::cout << "\033[33m" << ex.what() << "\033[m" << std::endl;
	}
	return 0;
}
