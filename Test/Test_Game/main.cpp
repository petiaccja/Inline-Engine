#include "Game.hpp"

#include <BaseLibrary/Platform/Window.hpp>
#include <BaseLibrary/Timer.hpp>

#undef interface // fucking shitty winapi as usual


using namespace inl;


void SetEvents(Window& window, Game& world, gxeng::IGraphicsEngine& engine) {
	window.OnResize += [&](ResizeEvent evt) -> void {
		if (evt.resizeMode == eResizeMode::MINIMIZED) {
			return;
		}
		engine.SetScreenSize(evt.clientSize.x, evt.clientSize.y);
		world.ResizeRender(evt.clientSize.x, evt.clientSize.y);
	};
}


int main() {
	try {
		Timer timer;
		Window window{ "Test game" };
		ModuleCollection modules{ window.GetNativeHandle() };
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
	catch (std::exception& ex) {
		std::cout << ex.what() << std::endl;
	}
	return 0;
}
