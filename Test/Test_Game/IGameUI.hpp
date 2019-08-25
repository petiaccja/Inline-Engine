#pragma once

#include <GuiEngine/Board.hpp>


class IGameUI {
public:
	virtual void SetBoard(inl::gui::Board& board) = 0;
	virtual void SetResolution(inl::Vec2u boardSize) = 0;
	virtual void Reset() = 0;
};