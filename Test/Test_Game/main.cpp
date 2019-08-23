#include "AssetCacheCollection.hpp"
#include "DebugInfoFrame.hpp"
#include "Game.hpp"
#include "ModuleCollection.hpp"
#include "UserInterface.hpp"

#include <BaseLibrary/Platform/Window.hpp>
#include <BaseLibrary/Timer.hpp>
#include <GameFoundationLibrary/Components/GraphicsMeshComponent.hpp>
#include <GameFoundationLibrary/Systems/GraphicsTransformSystem.hpp>
#include <GameFoundationLibrary/Systems/LinkTransformSystem.hpp>
#include <GameFoundationLibrary/Systems/RenderingSystem.hpp>
#include <GameLogic/Archive.hpp>
#include <GameLogic/ComponentFactory.hpp>
#include <GameLogic/World.hpp>
#include <GraphicsApi_D3D12/GxapiManager.hpp>
#include <GraphicsEngine_LL/DirectionalLight.hpp>
#include <GraphicsEngine_LL/GraphicsEngine.hpp>

#include <fstream>
#include <sstream>


using namespace inl;


struct Systems {
	gamelib::RenderingSystem renderingSystem;
	gamelib::LinkTransformSystem linkTransformSystem;
};


struct RenderScene {
	std::unique_ptr<gxeng::IScene> mainScene;
	std::unique_ptr<gxeng::IPerspectiveCamera> mainCamera;
	std::unique_ptr<gxeng::IScene> guiScene;
	std::unique_ptr<gxeng::ICamera2D> guiCamera;
};


Systems CreateSystems(const ModuleCollection& modules) {
	gamelib::RenderingSystem renderingSystem{ &modules.GetGraphicsEngine() };
	gamelib::LinkTransformSystem linkTransformSystem;

	return { std::move(renderingSystem), std::move(linkTransformSystem) };
}


void SetupGraphicsEngine(gxeng::IGraphicsEngine& graphicsEngine) {
	graphicsEngine.SetShaderDirectories({ INL_NODE_SHADER_DIRECTORY,
										  INL_MTL_SHADER_DIRECTORY,
										  "./Shaders",
										  "./Materials" });

	std::ifstream pipelineFile(INL_GAMEDATA "/Pipelines/new_forward_with_gui.json");
	if (!pipelineFile.is_open()) {
		throw FileNotFoundException("Failed to open pipeline JSON.");
	}
	std::string pipelineDesc((std::istreambuf_iterator<char>(pipelineFile)), std::istreambuf_iterator<char>());
	graphicsEngine.LoadPipeline(pipelineDesc);
}


RenderScene CreateRenderScene(gxeng::IGraphicsEngine& graphicsEngine) {
	RenderScene s;
	s.mainScene = graphicsEngine.CreateScene("MainScene");
	s.mainCamera = graphicsEngine.CreatePerspectiveCamera("MainCamera");
	s.guiScene = graphicsEngine.CreateScene("GuiScene");
	s.guiCamera = graphicsEngine.CreateCamera2D("GuiCamera");
	return s;
}


void SetupComponentFactories(gxeng::IGraphicsEngine& engine, const RenderScene& scene, const AssetCacheCollection& caches) {
	auto& componentFactory = game::ComponentFactory_Singleton::GetInstance();
	auto& graphicsMeshFactory = componentFactory.GetClassFactory<gamelib::GraphicsMeshComponent, gamelib::GraphicsMeshComponentFactory>();
	graphicsMeshFactory.SetCaches(&caches.GetGraphicsMeshCache(), &caches.GetMaterialCache());
	graphicsMeshFactory.SetEngine(&engine);
	graphicsMeshFactory.SetScenes({ scene.mainScene.get(), scene.guiScene.get() });
}


std::vector<game::Entity*> LoadScene(game::World& world) {
	gamelib::GraphicsMeshComponent comp;
	comp.properties.meshPath = "Models/Terrain/terrain.fbx";
	comp.properties.materialPath = "Models/Terrain/terrain.mtl";
	comp.properties.sceneName = "MainScene";
	std::stringstream ss;
	{
		cereal::JSONOutputArchive outAr(ss);
		outAr(comp);
	}
	game::InputArchive archive(std::in_place_type<cereal::JSONInputArchive>, ss);

	std::vector<game::Entity*> entities;
	entities.push_back(world.CreateEntity());
	game::ComponentFactory_Singleton::GetInstance().Create(*entities.back(), "GraphicsMeshComponent", archive);

	return entities;
}



int main() {
	try {
		Timer timer;
		Window window("Test game");
		Game game(window);

		timer.Start();
		while (!window.IsClosed()) {
			float frameTime = float(timer.Elapsed());
			timer.Reset();
			game.Update(frameTime);
		}
	}
	catch (std::exception& ex) {
		std::cout << ex.what() << std::endl;
	}
	return 0;


	try {
		Timer timer;
		Window window("Test game");

		ModuleCollection modules(window.GetNativeHandle());
		AssetCacheCollection assetCaches(modules.GetGraphicsEngine());

		SetupGraphicsEngine(modules.GetGraphicsEngine());

		game::World gameWorld;
		Systems systems = CreateSystems(modules);

		gameWorld.SetSystems({ &systems.linkTransformSystem,
							   &systems.renderingSystem });

		RenderScene renderScene = CreateRenderScene(modules.GetGraphicsEngine());
		SetupComponentFactories(modules.GetGraphicsEngine(), renderScene, assetCaches);

		std::unique_ptr<gxeng::DirectionalLight> sunLight = std::make_unique<gxeng::DirectionalLight>();
		sunLight->SetDirection(Vec3{ -0.2f, -0.3f, -0.6f }.Normalized());
		sunLight->SetColor({ 0.9f, 0.8f, 0.7f });
		renderScene.mainScene->GetEntities<gxeng::DirectionalLight>().Add(sunLight.get());

		UserInterface uiHost(modules.GetGraphicsEngine(), *renderScene.guiScene, *renderScene.guiCamera);
		DebugInfoFrame debugInfoFrame;
		debugInfoFrame.SetAdapterInfo(modules.GetGraphicsAdapter());
		debugInfoFrame.SetResolutionInfo(window.GetClientSize());
		uiHost.AddFrame(debugInfoFrame);
		debugInfoFrame.SetSize({ 500, 100 });

		renderScene.mainCamera->SetPosition({ 0, 0, 0.2 });
		renderScene.mainCamera->SetTarget({ 0, 1, 0 });
		renderScene.mainCamera->SetNearPlane(0.2f);
		renderScene.mainCamera->SetFarPlane(4000.f);

		window.OnResize += [&](ResizeEvent evt) {
			modules.GetGraphicsEngine().SetScreenSize(evt.clientSize.x, evt.clientSize.y);
			debugInfoFrame.SetResolutionInfo(evt.clientSize);
			uiHost.SetResolution(window.GetClientSize(), window.GetClientSize());
			uiHost[debugInfoFrame].SetPosition(Vec2(0, evt.clientSize.y) + Vec2(0.5f, -0.5f) * debugInfoFrame.GetSize());
			renderScene.mainCamera->SetFOVAspect(Deg2Rad(70.f), float(evt.clientSize.x) / float(evt.clientSize.y));
		};
		window.OnResize(ResizeEvent{ window.GetSize(), window.GetClientSize(), eResizeMode::RESTORED });

		auto entities = LoadScene(gameWorld);


		timer.Start();
		while (!window.IsClosed()) {
			float frameTime = float(timer.Elapsed());
			timer.Reset();
			gameWorld.Update(frameTime);
			uiHost.Update(frameTime);
			window.CallEvents();
		}
	}
	catch (std::exception& ex) {
		std::cout << "Error: " << ex.what() << std::endl;
	}
}
