#include "GameSceneFrame.hpp"

#include "Game.hpp"
#include "PauseMenuFrame.hpp"


GameSceneFrame::GameSceneFrame() {
	GetBackground().SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });
	OnKeyup += [this](Control* control, inl::eKey key) {
		if (key == inl::eKey::ESCAPE) {
			GetCompositor().ShowFrame<PauseMenuFrame>().SetSize({ 200, 350 });
			GetCompositor().GetBinding<PauseMenuFrame>().SetAnchors(true, true, true, true).SetResizing(false).MoveToFront();
		}
	};
}

void GameSceneFrame::UpdateStyle() {
	auto style = GetStyle();
	style.background = { 1.0f, 1.0f, 1.0f, 0.0f };
	GetBackground().SetStyle(style);
}
