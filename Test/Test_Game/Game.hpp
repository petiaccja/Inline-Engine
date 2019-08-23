#pragma once

#include "AssetCacheCollection.hpp"
#include "ModuleCollection.hpp"
#include "UserInterface.hpp"

#include <BaseLibrary/Platform/Window.hpp>
#include <GameLogic/ComponentFactory.hpp>
#include <GameLogic/System.hpp>
#include <GameLogic/World.hpp>
#include <GraphicsEngine/Scene/ICamera2D.hpp>
#include <GraphicsEngine/Scene/IPerspectiveCamera.hpp>
#include <GraphicsEngine/Scene/IScene.hpp>


class Game {
public:
	Game(inl::Window& window);

	void Update(float elapsed);

private:
	void CreateScenes();
	void CreateSystems();
	void SetupComponentFactories();
	void SetupRenderPipeline();
	void SetupGui();
	void SetupEvents();

	void OnResize(inl::ResizeEvent evt);

private:
	inl::Window* m_window = nullptr;
	ModuleCollection m_modules;
	AssetCacheCollection m_assetCaches;
	inl::game::ComponentFactory m_componentFactory;
	inl::game::World m_world;
	inl::gui::Board m_board;
	std::vector<std::unique_ptr<inl::gxeng::IScene>> m_scenes;
	std::vector<std::unique_ptr<inl::game::System>> m_systems;
	std::unique_ptr<inl::gxeng::IPerspectiveCamera> m_3dCamera;
	std::unique_ptr<inl::gxeng::ICamera2D> m_guiCamera;
	std::unique_ptr<inl::gxeng::IFont> m_font;
};
