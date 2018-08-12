#include <iostream>

#include <BaseLibrary/Logging_All.hpp>
#include <BaseLibrary/Platform/Window.hpp>
#include <GraphicsApi_D3D12/GxapiManager.hpp>
#include <GraphicsApi_LL/IGxapiManager.hpp>
#include <GraphicsApi_LL/IGraphicsApi.hpp>
#include <GraphicsEngine_LL/GraphicsEngine.hpp>

#include <GraphicsEngine_LL/Scene.hpp>
#include <GraphicsEngine_LL/PerspectiveCamera.hpp>
#include <GraphicsEngine_LL/DirectionalLight.hpp>
#include <GraphicsEngine_LL/MeshEntity.hpp>

#include <PhysicsEngine_Bullet/PhysicsEngine.hpp>
#include <PhysicsEngine_Bullet/Scene.hpp>
#include <PhysicsEngine_Bullet/MeshShape.hpp>
#include <PhysicsEngine_Bullet/RigidBody.hpp>

#include <AssetLibrary/AssetStore.hpp>


using namespace inl;


class GameScene {
public:
	GameScene(gxeng::GraphicsEngine* graphicsEngine, pxeng_bl::PhysicsEngine* physicsEngine);

	void LoadAssets();
protected:
	std::unique_ptr<gxeng::MeshEntity> CreateGraphicsEntity(std::string_view mesh, std::string_view material);
private:
	gxeng::GraphicsEngine* m_graphicsEngine;
	std::unique_ptr<gxeng::Scene> m_gxScene;
	std::unique_ptr<gxeng::PerspectiveCamera> m_camera;

	pxeng_bl::PhysicsEngine* m_physicsEngine;
	std::unique_ptr<pxeng_bl::Scene> m_pxScene;
	
	std::unique_ptr<gxeng::DirectionalLight> m_light;

	asset::AssetStore m_assetStore;

	std::vector<std::unique_ptr<gxeng::MeshEntity>> m_entities;
};


int main() {
	try {
		// Create logger and window.
		Logger logger;
		Window window{ "Physics test", {1024, 640}, false, false };

		// Create graphics API.
		std::unique_ptr<gxapi::IGxapiManager> gxapiManager(new gxapi_dx12::GxapiManager());

		auto adapters = gxapiManager->EnumerateAdapters();
		if (adapters.empty()) {
			throw RuntimeException("No suitable graphics adapter found.");
		}

		std::unique_ptr<gxapi::IGraphicsApi> graphicsApi(gxapiManager->CreateGraphicsApi(adapters[0].adapterId));


		// Create graphics engine.
		gxeng::GraphicsEngineDesc graphicsEngineDesc;
		graphicsEngineDesc.gxapiManager = gxapiManager.get();
		graphicsEngineDesc.graphicsApi = graphicsApi.get();
		graphicsEngineDesc.fullScreen = false;
		graphicsEngineDesc.width = window.GetClientSize().x;
		graphicsEngineDesc.height = window.GetClientSize().y;
		graphicsEngineDesc.logger = &logger;
		graphicsEngineDesc.targetWindow = window.GetNativeHandle();
		std::unique_ptr<gxeng::GraphicsEngine> graphicsEngine(new gxeng::GraphicsEngine(graphicsEngineDesc));

		// Load graphics pipeline.
		std::ifstream pipelineFile(INL_GAMEDATA R"(\Pipelines\new_forward.json)");
		if (!pipelineFile.is_open()) {
			throw FileNotFoundException("Failed to open pipeline JSON.");
		}
		std::string pipelineDesc((std::istreambuf_iterator<char>(pipelineFile)), std::istreambuf_iterator<char>());
		graphicsEngine->LoadPipeline(pipelineDesc);
		graphicsEngine->SetShaderDirectories({ INL_NODE_SHADER_DIRECTORY, INL_MTL_SHADER_DIRECTORY, "./Shaders", "./Materials" });

		// Create physics engine.
		std::unique_ptr<pxeng_bl::PhysicsEngine> physicsEngine(new pxeng_bl::PhysicsEngine());

		// Create scene and camera.
		GameScene gameScene(graphicsEngine.get(), physicsEngine.get());
		gameScene.LoadAssets();

		// Game loop.
		while (!window.IsClosed()) {
			window.CallEvents();
			graphicsEngine->Update(0.016f);
		}

		return 0;
	}
	catch (Exception& ex) {
		using namespace std;
		cout << "Unhandled exception occured." << endl;
		cout << "MESSAGE:" << ex.what() << endl;
		cout << "STACK TRACE:" << endl;
		ex.PrintStackTrace(cout);
		cout << "PRESS ENTER TO QUIT..." << endl;
		cin.get();
	}
}


inline GameScene::GameScene(gxeng::GraphicsEngine * graphicsEngine, pxeng_bl::PhysicsEngine * physicsEngine)
	: m_graphicsEngine(graphicsEngine), m_physicsEngine(physicsEngine), m_assetStore(graphicsEngine)
{
	m_assetStore.AddSourceDirectory(INL_GAMEDATA);

	m_gxScene.reset(m_graphicsEngine->CreateScene("MainScene"));
	m_camera.reset(m_graphicsEngine->CreatePerspectiveCamera("MainCamera"));
	m_light.reset(new gxeng::DirectionalLight());

	m_pxScene.reset(m_physicsEngine->CreateScene());

	m_light->SetDirection(Vec3{ -1, -2, -3 }.Normalized());
	m_light->SetColor({ 0.7f, 0.8f, 0.9f });

	m_gxScene->GetEntities<gxeng::DirectionalLight>().Add(m_light.get());

	m_camera->SetTargeted(true);
	m_camera->SetPosition({ 10, 10, 10 });
	m_camera->SetTarget({ 0,0,0 });
	m_camera->SetUpVector({ 0,0,1 });
}


void GameScene::LoadAssets() {
	auto terrain = CreateGraphicsEntity("Models/Terrain/terrain.fbx", "models/Terrain/terrain.mtl");
	m_gxScene->GetEntities<gxeng::MeshEntity>().Add(terrain.get());
	m_entities.push_back(std::move(terrain));
}


std::unique_ptr<gxeng::MeshEntity> GameScene::CreateGraphicsEntity(std::string_view mesh, std::string_view material) {
	std::unique_ptr<gxeng::MeshEntity> entity(m_graphicsEngine->CreateMeshEntity());

	auto meshResource = m_assetStore.LoadMesh(mesh);
	auto materialResource = m_assetStore.LoadMaterial(material);

	entity->SetMesh(meshResource.get());
	entity->SetMaterial(materialResource.get());

	return entity;
}
