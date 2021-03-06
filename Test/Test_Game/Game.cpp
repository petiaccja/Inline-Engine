#include "Game.hpp"

#include "ActionHook.hpp"
#include "CameraMoveSource.hpp"
#include "CameraMoveSystem.hpp"
#include "LevelSystem.hpp"
#include "MainMenuFrame.hpp"
#include "TestLevelSystem.hpp"
#include "UserInputSystem.hpp"
#include "UserInterfaceSystem.hpp"

#include <GameFoundationLibrary/Components/GraphicsMeshComponent.hpp>
#include <GameFoundationLibrary/Components/PerspectiveCameraComponent.hpp>
#include <GameFoundationLibrary/Systems/HeightmapTransformSystem.hpp>
#include <GameFoundationLibrary/Systems/LinkTransformSystem.hpp>
#include <GameFoundationLibrary/Systems/MeshTransformSystem.hpp>
#include <GameFoundationLibrary/Systems/RenderingSystem.hpp>

#include <fstream>


#define RENDER_PIPELINE_FULL_3D "forward_heightmap_with_gui.json"
#define RENDER_PIPELINE_GUI_ONLY "gui_only.json"


Game::Game(const EngineCollection& modules, inl::Window& window)
	: m_window(window), m_engines(modules), m_modules(std::make_shared<inl::DynamicTuple>()) {
	auto graphicsModule = std::make_shared<inl::gamelib::GraphicsModule>(modules.GetGraphicsEngine(), INL_GAMEDATA);
	graphicsModule->GetOrCreateScene("MainScene"); // Make it in advance
	m_modules->Insert(graphicsModule);

	InitSimulation();
	InitRenderPaths();
	UpdateRenderPipeline();

	window.OnResize += [this](inl::ResizeEvent evt) {
		SetSceneCameraARs(evt.clientSize.x / float(evt.clientSize.y));
	};
}


void Game::Update(float elapsed) {
	UpdateRenderPipeline();
	m_simulation.Run(m_scene, elapsed);
}


void Game::InitSimulation() {
	m_simulation.systems = {
		UserInterfaceSystem{ m_engines, m_window },
		UserInputSystem{},
		TestLevelSystem{ m_modules },
		LevelSystem{ m_modules },
		CameraMoveSystem{},
		inl::gamelib::LinkTransformSystem{},
		inl::gamelib::MeshTransformSystem{},
		inl::gamelib::HeightmapTransformSystem{},
		inl::gamelib::RenderingSystem{ &m_engines.GetGraphicsEngine() },
	};

	m_simulation.hooks = {
		ActionHook{},
	};

	auto& userInputSystem = dynamic_cast<UserInputSystem&>(*std::find_if(m_simulation.systems.begin(), m_simulation.systems.end(), [](auto& sys) { return typeid(sys) == typeid(UserInputSystem); }));

	userInputSystem.Insert(CameraMoveSource{});
}


void Game::InitRenderPaths() {
	auto& engine = m_engines.GetGraphicsEngine();
	engine.SetShaderDirectories({ INL_NODE_SHADER_DIRECTORY,
								  INL_MTL_SHADER_DIRECTORY,
								  "./Shaders",
								  "./Materials" });
}


void Game::LoadRenderPipeline(std::string_view path) {
	std::ifstream pipelineFile(std::filesystem::path(INL_GAMEDATA) / path);
	if (!pipelineFile.is_open()) {
		throw inl::FileNotFoundException("Failed to open pipeline JSON.");
	}
	std::string pipelineDesc((std::istreambuf_iterator<char>(pipelineFile)), std::istreambuf_iterator<char>());
	m_engines.GetGraphicsEngine().LoadPipeline(pipelineDesc);
}


std::vector<inl::gxeng::IPerspectiveCamera*> Game::GetSceneCameras() const {
	std::vector<inl::gxeng::IPerspectiveCamera*> cameras;
	for (auto& matrix : m_scene.GetSchemeSets({ typeid(inl::gamelib::PerspectiveCameraComponent) })) {
		inl::game::ComponentRange<const inl::gamelib::PerspectiveCameraComponent> range{ matrix.get().GetMatrix() };
		for (auto [camera] : range) {
			cameras.push_back(camera.entity.get());
		}
	}
	return cameras;
}


void Game::SetSceneCameraARs(float aspectRatio) const {
	auto cameras = GetSceneCameras();
	for (auto camera : cameras) {
		float fovh = camera->GetFOVHorizontal();
		camera->SetFOVAspect(fovh, aspectRatio);
	}
}


void Game::UpdateRenderPipeline() {
	auto cameras = GetSceneCameras();
	bool cameraFound = false;
	for (auto& camera : cameras) {
		cameraFound = cameraFound || camera->GetName() == "MainCamera";
	}

	eRenderMode desiredMode = cameraFound ? eRenderMode::FULL : eRenderMode::GUI;

	if (m_renderMode != desiredMode) {
		switch (desiredMode) {
			case eRenderMode::FULL: LoadRenderPipeline("Pipelines/" RENDER_PIPELINE_FULL_3D); break;
			case eRenderMode::GUI: LoadRenderPipeline("Pipelines/" RENDER_PIPELINE_GUI_ONLY); break;
		}
		m_renderMode = desiredMode;
	}
}
