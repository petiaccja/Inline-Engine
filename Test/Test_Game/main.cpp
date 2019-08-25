#include "Game.hpp"
#include "TestGameLevel.hpp"
#include "TestGameUI.hpp"

#include <BaseLibrary/Platform/Window.hpp>
#include <BaseLibrary/Timer.hpp>


using namespace inl;


int main() {
	try {
		Timer timer;
		Window window("Test game");
		Game game(window);
		TestGameUI gameUi;
		TestGameLevel gameLevel;

		game.SetGameUi(gameUi);
		game.SetGameLevel(gameLevel);

		timer.Start();
		while (!window.IsClosed()) {
			float frameTime = float(timer.Elapsed());
			timer.Reset();
			game.Update(frameTime);
		}
	}
	catch (std::exception& ex) {
		std::cout << ex.what() << std::endl;
	}
	return 0;
}
