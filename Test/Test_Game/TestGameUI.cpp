#include "TestGameUI.hpp"


TestGameUI::TestGameUI() {
	m_mainLayout.SetReferencePoint(inl::gui::AbsoluteLayout::eRefPoint::TOPLEFT);
	m_mainLayout.SetYDown(true);

	m_debugInfoFrame.SetSize({ 400, 200 });

	m_mainLayout.AddChild(m_debugInfoFrame);
	m_mainLayout[&m_debugInfoFrame].SetPosition(m_debugInfoFrame.GetSize() / 2.0f);
}


void TestGameUI::SetBoard(inl::gui::Board& board) {
	m_board = &board;
	m_board->AddChild(m_mainLayout);
}


void TestGameUI::SetResolution(inl::Vec2u boardSize) {
	m_mainLayout.SetSize(inl::Vec2(boardSize));
	m_mainLayout.SetPosition(inl::Vec2(boardSize) / 2.0f);
}


void TestGameUI::Reset() {
	assert(m_board);
	m_board->RemoveChild(&m_mainLayout);
	m_board = nullptr;
}
