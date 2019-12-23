#include "Game.hpp"

#include "UserInterfaceSystem.hpp"
#include "WindowEventSystem.hpp"

#include <GameFoundationLibrary/Components/PerspectiveCameraComponent.hpp>
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

void Game::operator()(ResizeScreenAction action) {
	ResizeRender(action.width, action.height);
}

void Game::operator()(LoadLevelAction action) {
	m_scene.Clear();
	auto level = action.Level(m_scene);
	//level->Load(inl::game::ComponentFactory_Singleton::GetInstance());
	
}

void Game::ResizeRender(int width, int height) {
	// Set aspect ratios for cameras.
	for (auto& matrix : m_scene.GetSchemeSets({ typeid(inl::gamelib::PerspectiveCameraComponent) })) {
		inl::game::ComponentRange<inl::gamelib::PerspectiveCameraComponent> range{ matrix.get().GetMatrix() };
		for (auto [camera] : range) {
			float fovh = camera.entity->GetFOVHorizontal();
			camera.entity->SetFOVAspect(fovh, float(width) / float(height));
		}		
	}
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
