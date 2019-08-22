#include "Game.hpp"

#include "UserInterfaceSystem.hpp"
#include "WindowEventSystem.hpp"

#include <GameFoundationLibrary/Components/DirectionalLightComponent.hpp>
#include <GameFoundationLibrary/Components/GraphicsMeshComponent.hpp>
#include <GameFoundationLibrary/Systems/LinkTransformSystem.hpp>
#include <GameFoundationLibrary/Systems/RenderingSystem.hpp>

#include <fstream>


Game::Game(inl::Window& window)
	: m_modules(window.GetNativeHandle()),
	  m_assetCaches(m_modules.GetGraphicsEngine()),
	  m_componentFactory(inl::game::ComponentFactory_Singleton::GetInstance()),
	  m_window(&window) {
	CreateScenes();
	CreateSystems();
	SetupComponentFactories();
	SetupRenderPipeline();
}


void Game::Update(float elapsed) {
	world.Update(elapsed);
}


void Game::CreateScenes() {
	auto& engine = m_modules.GetGraphicsEngine();
	m_scenes.push_back(std::unique_ptr<inl::gxeng::IScene>(engine.CreateScene("MainScene")));
	m_scenes.push_back(std::unique_ptr<inl::gxeng::IScene>(engine.CreateScene("GuiScene")));
	m_guiCamera.reset(engine.CreateCamera2D("GuiCamera"));
	m_3dCamera.reset(engine.CreatePerspectiveCamera("MaiNCamera"));
}


void Game::CreateSystems() {
	auto windowEventSystem = std::make_unique<WindowEventSystem>();
	auto userInterfaceSystem = std::make_unique<UserInterfaceSystem>();

	windowEventSystem->SetWindows({ m_window });
	userInterfaceSystem->SetBoards({ &m_board });

	m_systems.push_back(std::move(windowEventSystem));
	m_systems.push_back(std::move(userInterfaceSystem));
	m_systems.push_back(std::make_unique<inl::gamelib::LinkTransformSystem>());
	m_systems.push_back(std::make_unique<inl::gamelib::RenderingSystem>(&m_modules.GetGraphicsEngine()));
}


void Game::SetupComponentFactories() {
	std::vector<inl::gxeng::IScene*> scenes;
	for (auto& s : m_scenes) {
		scenes.push_back(s.get());
	}
	// TODO: use ranges, std:transforms is shite.
	// std::transform(m_scenes.begin(), m_scenes.end(), std::back_inserter(scenes), [](auto s) { return s.get(); });
	// scenes = { m_scenes | view::transform([](auto s) { return s.get(); }) };

	auto& graphicsMeshFactory = m_componentFactory.GetClassFactory<inl::gamelib::GraphicsMeshComponent, inl::gamelib::GraphicsMeshComponentFactory>();
	graphicsMeshFactory.SetEngine(&m_modules.GetGraphicsEngine());
	graphicsMeshFactory.SetCaches(&m_assetCaches.GetGraphicsMeshCache(), &m_assetCaches.GetMaterialCache());
	graphicsMeshFactory.SetScenes(scenes);

	auto& directionalLightFactory = m_componentFactory.GetClassFactory<inl::gamelib::DirectionalLightComponent, inl::gamelib::DirectionalLightComponentFactory>();
	directionalLightFactory.SetEngine(&m_modules.GetGraphicsEngine());
	directionalLightFactory.SetScenes(scenes);
}


void Game::SetupRenderPipeline() {
	auto& engine = m_modules.GetGraphicsEngine();
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


void Game::SetupGui() {
	auto& engine = m_modules.GetGraphicsEngine();
	auto font = engine.CreateFont();
}
