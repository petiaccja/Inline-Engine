#include "Game.hpp"

#include "UserInterfaceSystem.hpp"
#include "WindowEventSystem.hpp"

#include <GameFoundationLibrary/Components/GraphicsMeshComponent.hpp>
#include <GameFoundationLibrary/Components/PerspectiveCameraComponent.hpp>
#include <GameFoundationLibrary/Systems/LinkTransformSystem.hpp>
#include <GameFoundationLibrary/Systems/RenderingSystem.hpp>

#include <fstream>


#define RENDER_PIPELINE_FULL_3D "forward_with_gui.json"
#define RENDER_PIPELINE_GUI_ONLY "gui_only.json"

Game::Game(const ModuleCollection& modules, inl::Window& window)
	: m_graphicsModule(modules.GetGraphicsEngine(), INL_GAMEDATA), m_window(window) {
	CreateSystems(modules);
	SetupRenderPipeline(modules);
}

void Game::Update(float elapsed) {
	m_simulation.Run(m_scene, elapsed);
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
	m_simulation.PushBack(UserInterfaceSystem{ modules, m_window });
	m_simulation.PushBack(inl::gamelib::LinkTransformSystem{});
	m_simulation.PushBack(inl::gamelib::RenderingSystem{ &modules.GetGraphicsEngine() });
}

void Game::SetupRenderPipeline(const ModuleCollection& modules) {
	auto& engine = modules.GetGraphicsEngine();
	engine.SetShaderDirectories({ INL_NODE_SHADER_DIRECTORY,
								  INL_MTL_SHADER_DIRECTORY,
								  "./Shaders",
								  "./Materials" });

	std::ifstream pipelineFile(INL_GAMEDATA "/Pipelines/" RENDER_PIPELINE_GUI_ONLY);
	if (!pipelineFile.is_open()) {
		throw inl::FileNotFoundException("Failed to open pipeline JSON.");
	}
	std::string pipelineDesc((std::istreambuf_iterator<char>(pipelineFile)), std::istreambuf_iterator<char>());
	engine.LoadPipeline(pipelineDesc);
}
