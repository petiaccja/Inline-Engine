#pragma once

#include "DebugInfoFrame.hpp"
#include "IGameUI.hpp"
#include "MainMenuFrame.hpp"

#include <GuiEngine/AbsoluteLayout.hpp>


class TestGameUI : public IGameUI {
public:
	TestGameUI();

	void SetBoard(inl::gui::Board& board) override;
	void SetResolution(inl::Vec2u boardSize) override;
	void Reset() override;

private:
	inl::gui::Board* m_board = nullptr;
	inl::gui::AbsoluteLayout m_mainLayout;
	DebugInfoFrame m_debugInfoFrame;
	MainMenuFrame m_mainMenuFrame;
};
