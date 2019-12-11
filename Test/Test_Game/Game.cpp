#include "Game.hpp"

#include "UserInterfaceSystem.hpp"
#include "WindowEventSystem.hpp"

#include <GameFoundationLibrary/Components/DirectionalLightComponent.hpp>
#include <GameFoundationLibrary/Components/GraphicsMeshComponent.hpp>
#include <GameFoundationLibrary/Systems/LinkTransformSystem.hpp>
#include <GameFoundationLibrary/Systems/RenderingSystem.hpp>

#include <fstream>


Game::Game(const ModuleCollection& modules)
	: m_graphicsModule(modules.GetGraphicsEngine(), INL_GAMEDATA) {
	CreateSystems(modules);
	SetupRenderPipeline(modules);
}

void Game::Update(float elapsed) {
	m_simulation.Run(m_scene, elapsed);
}

void Game::ResizeRender(int width, int height) {
	// TODO: resize all cameras???
}


void Game::CreateSystems(const ModuleCollection& modules) {
	m_simulation.PushBack(inl::gamelib::LinkTransformSystem{});
	m_simulation.PushBack(inl::gamelib::RenderingSystem{ &modules.GetGraphicsEngine() });
}

void Game::SetupRenderPipeline(const ModuleCollection& modules) {
	auto& engine = modules.GetGraphicsEngine();
	engine.SetShaderDirectories({ INL_NODE_SHADER_DIRECTORY,
								  INL_MTL_SHADER_DIRECTORY,
								  "./Shaders",
								  "./Materials" });

	std::ifstream pipelineFile(INL_GAMEDATA "/Pipelines/new_forward_with_gui.json");
	if (!pipelineFile.is_open()) {
		throw inl::FileNotFoundException("Failed to open pipeline JSON.");
	}
	std::string pipelineDesc((std::istreambuf_iterator<char>(pipelineFile)), std::istreambuf_iterator<char>());
	engine.LoadPipeline(pipelineDesc);
}
