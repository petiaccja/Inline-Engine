#pragma once

#include "ActionHeap.hpp"
#include "EngineCollection.hpp"

#include <BaseLibrary/Container/DynamicTuple.hpp>
#include <GameFoundationLibrary/Modules/GraphicsModule.hpp>
#include <GameLogic/Scene.hpp>
#include <GameLogic/Simulation.hpp>


class Game {
	enum class eRenderMode {
		FULL,
		GUI,
		OFF,
	};

public:
	Game(const EngineCollection& modules, inl::Window& window);
	void Update(float elapsed);

private:
	void InitSimulation();
	void InitRenderPaths();
	void LoadRenderPipeline(std::string_view path);

	std::vector<inl::gxeng::IPerspectiveCamera*> GetSceneCameras() const;
	void SetSceneCameraARs(float aspectRatio) const;

	void UpdateRenderPipeline();

private:
	inl::game::Scene m_scene;
	inl::game::Simulation m_simulation;
	eRenderMode m_renderMode = eRenderMode::OFF;

	const EngineCollection& m_engines;
	std::shared_ptr<inl::DynamicTuple> m_modules;
	inl::Window& m_window;
};
