#pragma once

#include "Actions.hpp"
#include "ModuleCollection.hpp"

#include <BaseLibrary/ActionSystem/ActionSystem.hpp>
#include <GameFoundationLibrary/Modules/GraphicsModule.hpp>
#include <GameLogic/Scene.hpp>
#include <GameLogic/Simulation.hpp>


// TODO:
// How does Game receive action to load level?
// - just a string?
// - maybe an ILevel object?
// - a function object taking Scene&?
// - the LoadLevelAction could be a cultured functor?

class Game : public inl::ActionListener<Game, ResizeScreenAction, LoadLevelAction> {
public:
	Game(const ModuleCollection& modules);

	void Update(float elapsed);

	void operator()(ResizeScreenAction action);
	void operator()(LoadLevelAction action);

	void ResizeRender(int width, int height);

private:
	void CreateSystems(const ModuleCollection& modules);
	void SetupRenderPipeline(const ModuleCollection& modules);

private:
	inl::game::Scene m_scene;
	inl::game::Simulation m_simulation;
	inl::gamelib::GraphicsModule m_graphicsModule;
};
