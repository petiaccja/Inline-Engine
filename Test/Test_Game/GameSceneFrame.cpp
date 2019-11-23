#include "GameSceneFrame.hpp"

#include "GameWorld.hpp"


GameSceneFrame::GameSceneFrame() {
	GetBackground().SetColor({ 0.1f, 0.6f, 0.1f, 0.5f });
}


void GameSceneFrame::SetGameWorld(GameWorld& gameWorld) {
	m_gameWorld = &gameWorld;
}

void GameSceneFrame::Start(std::unique_ptr<ILevel> level) {	
	assert(m_gameWorld);
	m_level = std::move(level);
	inl::game::Scene newWorld = m_level->Initialize(m_gameWorld->GetComponentFactory(), { 0, 0, 0 });
	inl::game::Scene& oldWorld = m_gameWorld->GetWorld();

	oldWorld += std::move(newWorld);
}
