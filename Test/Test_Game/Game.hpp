#pragma once

#include "ModuleCollection.hpp"

#include <GameFoundationLibrary/Modules/GraphicsModule.hpp>
#include <GameLogic/Scene.hpp>
#include <GameLogic/Simulation.hpp>



class Game {
public:
	Game(const ModuleCollection& modules, inl::Window& window);

	void Update(float elapsed);

	void ResizeRender(int width, int height);

private:
	void CreateSystems(const ModuleCollection& modules);
	void SetupRenderPipeline(const ModuleCollection& modules);

private:
	inl::game::Scene m_scene;
	inl::game::Simulation m_simulation;
	inl::gamelib::GraphicsModule m_graphicsModule;
	inl::Window& m_window;
};
