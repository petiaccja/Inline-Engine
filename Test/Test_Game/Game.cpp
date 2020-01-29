#include "Game.hpp"

#include "CameraMoveSystem.hpp"
#include "MainMenuFrame.hpp"
#include "TestLevelSystem.hpp"
#include "UserInterfaceSystem.hpp"

#include <GameFoundationLibrary/Components/GraphicsMeshComponent.hpp>
#include <GameFoundationLibrary/Components/PerspectiveCameraComponent.hpp>
#include <GameFoundationLibrary/Systems/LinkTransformSystem.hpp>
#include <GameFoundationLibrary/Systems/RenderingSystem.hpp>

#include <fstream>


#define RENDER_PIPELINE_FULL_3D "forward_heightmap_with_gui.json"
#define RENDER_PIPELINE_GUI_ONLY "gui_only.json"


Game::Game(const EngineCollection& modules, inl::Window& window)
	: m_graphicsModule(modules.GetGraphicsEngine(), INL_GAMEDATA), m_window(window), m_engines(modules) {
	InitSimulation();
	InitRenderPaths();
	m_modules.Insert(&m_graphicsModule);
	UpdateRenderPipeline();

	window.OnResize += [this](inl::ResizeEvent evt) {
		SetSceneCameraARs(evt.clientSize.x / float(evt.clientSize.y));
	};
}


void Game::Update(float elapsed) {
	UpdateRenderPipeline();
	m_simulation.Run(m_scene, elapsed);
	m_actionHeap->Clear();
}


void Game::InitSimulation() {
	m_actionHeap = std::make_shared<ActionHeap>();
	
	m_simulation.PushBack(UserInterfaceSystem{ m_engines, m_window, m_actionHeap });
	m_simulation.PushBack(TestLevelSystem{});
	m_simulation.PushBack(CameraMoveSystem{m_actionHeap});
	m_simulation.PushBack(inl::gamelib::LinkTransformSystem{});
	m_simulation.PushBack(inl::gamelib::RenderingSystem{ &m_engines.GetGraphicsEngine() });

	m_simulation.Get<UserInterfaceSystem>().GetCompositor().GetFrame<MainMenuFrame>().OnStart += [this, &testLevelSystem = m_simulation.Get<TestLevelSystem>()] {
		testLevelSystem.LoadAsync(inl::game::ComponentFactory_Singleton::GetInstance(), m_modules);
	};
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
	m_graphicsModule.GetOrCreateScene("MainScene");

	eRenderMode desiredMode = cameraFound ? eRenderMode::FULL : eRenderMode::GUI;

	if (m_renderMode != desiredMode) {
		switch (desiredMode) {
			case eRenderMode::FULL: LoadRenderPipeline("Pipelines/" RENDER_PIPELINE_FULL_3D); break;
			case eRenderMode::GUI: LoadRenderPipeline("Pipelines/" RENDER_PIPELINE_GUI_ONLY); break;
		}
		m_renderMode = desiredMode;
	}
}
