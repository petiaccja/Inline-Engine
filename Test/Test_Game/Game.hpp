#pragma once

#include "AssetCacheCollection.hpp"
#include "ModuleCollection.hpp"

#include <BaseLibrary/Platform/Window.hpp>
#include <GameLogic/System.hpp>
#include <GameLogic/World.hpp>
#include <GraphicsEngine/Scene/ICamera2D.hpp>
#include <GraphicsEngine/Scene/IPerspectiveCamera.hpp>
#include <GraphicsEngine/Scene/IScene.hpp>


class Game {
public:
	Game(inl::Window& window);

private:
private:
	ModuleCollection m_modules;
	AssetCacheCollection m_assetCaches;
	std::vector<std::unique_ptr<inl::gxeng::IScene>> m_scenes;
	std::vector<std::unique_ptr<inl::game::System>> m_systems;
	std::unique_ptr<inl::gxeng::IPerspectiveCamera> m_3dCamera;
	std::unique_ptr<inl::gxeng::ICamera2D> m_guiCamera;
	inl::game::World world;
};
