#include "GameSceneFrame.hpp"

#include "Game.hpp"


GameSceneFrame::GameSceneFrame() {
	GetBackground().SetColor({ 0.1f, 0.6f, 0.1f, 0.5f });
}


void GameSceneFrame::SetGameWorld(Game& gameWorld) {
	m_gameWorld = &gameWorld;
}

void GameSceneFrame::Start(std::unique_ptr<ILevel> level) {	

}
