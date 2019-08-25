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
	SetupGui();
	SetupEvents();
}


void Game::Update(float elapsed) {
	m_world.Update(elapsed);
}


void Game::CreateScenes() {
	auto& engine = m_modules.GetGraphicsEngine();
	m_scenes.push_back(std::unique_ptr<inl::gxeng::IScene>(engine.CreateScene("MainScene")));
	m_scenes.push_back(std::unique_ptr<inl::gxeng::IScene>(engine.CreateScene("GuiScene")));
	m_guiCamera = engine.CreateCamera2D("GuiCamera");
	m_3dCamera = engine.CreatePerspectiveCamera("MainCamera");
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

	std::vector<inl::game::System*> systems;
	for (const auto& s : m_systems) {
		systems.push_back(s.get());
	}
	m_world.SetSystems(std::move(systems));
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

	auto m_font = engine.CreateFont();
	std::ifstream fontFile;
	fontFile.open(R"(C:\Windows\Fonts\calibri.ttf)", std::ios::binary);
	m_font->LoadFile(fontFile);

	inl::gui::GraphicsContext ctx;
	ctx.engine = &engine;
	ctx.scene = m_scenes[1].get();
	m_board.SetDrawingContext(ctx);
	m_board.SetDepth(0.0f);
	inl::gui::ControlStyle style = inl::gui::ControlStyle::Dark(inl::Window::GetWindows10AccentColor().value_or(inl::ColorF{ 0.8f, 0.2f, 0.2f, 1.0f }));
	style.font = m_font.get();
	m_board.SetStyle(style);
}


void Game::SetupEvents() {
	m_window->OnResize += inl::Delegate<void(inl::ResizeEvent)>{ &Game::OnResize, this };
}


void Game::SetGameUi(IGameUI& gameUi) {
	if (m_gameUi) {
		m_gameUi->Reset();
	}
	m_gameUi = &gameUi;
	m_gameUi->SetBoard(m_board);
}


void Game::SetGameLevel(IGameLevel& gameLevel) {
	if (m_gameLevel) {
		m_gameLevel->Reset();
	}
	m_gameLevel = &gameLevel;
	m_gameLevel->SetWorld(m_world);
}


void Game::OnResize(inl::ResizeEvent evt) {
	const auto& resolution = evt.clientSize;

	m_modules.GetGraphicsEngine().SetScreenSize(evt.clientSize.x, evt.clientSize.y);

	m_board.SetCoordinateMapping({ 0.f, (float)resolution.x, (float)resolution.y, 0.f }, { 0.f, (float)resolution.x, 0.f, (float)resolution.y });
	m_guiCamera->SetPosition(inl::Vec2(resolution) / 2.0f);
	m_guiCamera->SetExtent(resolution);
	m_guiCamera->SetRotation(0.0f);
	m_guiCamera->SetVerticalFlip(false);
}
