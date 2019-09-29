#pragma once

#include "ModuleCollection.hpp"

#include <GameLogic/ComponentFactory.hpp>
#include <GraphicsEngine/Scene/ICamera2D.hpp>
#include <GraphicsEngine/Scene/IScene.hpp>
#include <GuiEngine/Board.hpp>

 

class GameInterface {
public:
	GameInterface(const ModuleCollection& modules);

	inl::gui::Board& GetBoard();
	inl::gxeng::ICamera2D& GetCamera();

private:
	std::unique_ptr<inl::gxeng::IFont> m_font;
	std::unique_ptr<inl::gxeng::IScene> m_scene;
	std::unique_ptr<inl::gxeng::ICamera2D> m_camera;
	inl::gui::Board m_board;
};