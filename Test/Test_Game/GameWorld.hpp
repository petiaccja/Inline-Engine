#pragma once

#include "AssetCacheCollection.hpp"
#include "ModuleCollection.hpp"

#include <GameLogic/ComponentFactory.hpp>
#include <GameLogic/System.hpp>
#include <GameLogic/World.hpp>
#include <GraphicsEngine/Scene/IPerspectiveCamera.hpp>
#include <GraphicsEngine/Scene/IScene.hpp>


class GameWorld {
public:
	GameWorld(const ModuleCollection& modules, const AssetCacheCollection& assetCaches);

	inl::game::World& GetWorld();
	inl::gxeng::IPerspectiveCamera& GetCamera();
	inl::game::ComponentFactory GetComponentFactory();
	
private:
	void CreateSystems(const ModuleCollection& modules);
	void SetupComponentFactories(const ModuleCollection& modules, const AssetCacheCollection& assetCaches);
	void SetupRenderPipeline(const ModuleCollection& modules);

private:
	inl::game::ComponentFactory m_componentFactory;
	inl::game::World m_world;
	std::unique_ptr<inl::gxeng::IPerspectiveCamera> m_camera;
	std::vector<std::unique_ptr<inl::game::System>> m_systems;
	std::unique_ptr<inl::gxeng::IScene> m_scene;
};

