#include "UserInterfaceSystem.hpp"


void UserInterfaceSystem::Update(float elapsed) {
	for (auto board : m_boards) {
		board->Update(elapsed);
	}
}


void UserInterfaceSystem::SetBoards(std::vector<inl::gui::Board*> boards) {
	m_boards = std::move(boards);
}
