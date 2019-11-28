#include "GameWorld.hpp"

#include "UserInterfaceSystem.hpp"
#include "WindowEventSystem.hpp"

#include <GameFoundationLibrary/Components/DirectionalLightComponent.hpp>
#include <GameFoundationLibrary/Components/GraphicsMeshComponent.hpp>
#include <GameFoundationLibrary/Systems/LinkTransformSystem.hpp>
#include <GameFoundationLibrary/Systems/RenderingSystem.hpp>

#include <fstream>


GameWorld::GameWorld(const ModuleCollection& modules, const AssetCacheCollection& assetCaches) {
	m_camera = modules.GetGraphicsEngine().CreatePerspectiveCamera("MainCamera");
	m_scene = modules.GetGraphicsEngine().CreateScene("MainScene");
	m_componentFactory = inl::game::ComponentFactory_Singleton::GetInstance();
	CreateSystems(modules);
	SetupRenderPipeline(modules);
}


inl::game::Scene& GameWorld::GetScene() {
	return m_world;
}

inl::game::Simulation& GameWorld::GetSimulation() {
	return m_simulation;
}


inl::gxeng::IPerspectiveCamera& GameWorld::GetCamera() {
	return *m_camera;
}

inl::game::ComponentFactory& GameWorld::GetComponentFactory() {
	return m_componentFactory;
}


void GameWorld::CreateSystems(const ModuleCollection& modules) {
	auto windowEventSystem = std::make_unique<WindowEventSystem>();
	auto userInterfaceSystem = std::make_unique<UserInterfaceSystem>();

	m_simulation.PushBack(WindowEventSystem{});
	m_simulation.PushBack(UserInterfaceSystem{});
	m_simulation.PushBack(inl::gamelib::LinkTransformSystem{});
	m_simulation.PushBack(inl::gamelib::RenderingSystem{ &modules.GetGraphicsEngine() });
}

void GameWorld::SetupRenderPipeline(const ModuleCollection& modules) {
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
