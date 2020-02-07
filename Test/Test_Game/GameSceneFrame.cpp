#include "GameSceneFrame.hpp"

#include "Game.hpp"


GameSceneFrame::GameSceneFrame() {
	GetBackground().SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });
}

void GameSceneFrame::UpdateStyle() {
	auto style = GetStyle();
	style.background = { 1.0f, 1.0f, 1.0f, 0.0f };
	GetBackground().SetStyle(style);
}
