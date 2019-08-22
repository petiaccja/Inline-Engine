#pragma once

#include <GameLogic/System.hpp>
#include <GuiEngine/Board.hpp>


class UserInterfaceSystem : public inl::game::SpecificSystem<UserInterfaceSystem> {
public:
	void Update(float elapsed) override;
	void SetBoards(std::vector<inl::gui::Board*> boards);

private:
	std::vector<inl::gui::Board*> m_boards;
};