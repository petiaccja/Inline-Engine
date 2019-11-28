#pragma once

#include "AssetCacheCollection.hpp"
#include "ModuleCollection.hpp"

#include <GameLogic/ComponentFactory.hpp>
#include <GameLogic/System.hpp>
#include <GameLogic/Scene.hpp>
#include <GraphicsEngine/Scene/IPerspectiveCamera.hpp>
#include <GraphicsEngine/Scene/IScene.hpp>
#include "GameLogic/Simulation.hpp"


class GameWorld {
public:
	GameWorld(const ModuleCollection& modules, const AssetCacheCollection& assetCaches);

	inl::game::Scene& GetScene();
	inl::game::Simulation& GetSimulation();	
	inl::gxeng::IPerspectiveCamera& GetCamera();
	inl::game::ComponentFactory& GetComponentFactory();
	
private:
	void CreateSystems(const ModuleCollection& modules);
	void SetupRenderPipeline(const ModuleCollection& modules);

private:
	inl::game::ComponentFactory m_componentFactory;
	inl::game::Scene m_world;
	inl::game::Simulation m_simulation;
	std::unique_ptr<inl::gxeng::IPerspectiveCamera> m_camera;
	std::unique_ptr<inl::gxeng::IScene> m_scene;
};

