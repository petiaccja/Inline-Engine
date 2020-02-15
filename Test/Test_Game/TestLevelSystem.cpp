#include "TestLevelSystem.hpp"

#include "LevelActions.hpp"

#include <BaseLibrary/Container/DynamicTuple.hpp>
#include <GameFoundationLibrary/Components/DirectionalLightComponent.hpp>
#include <GameFoundationLibrary/Components/GraphicsHeightmapComponent.hpp>
#include <GameFoundationLibrary/Components/GraphicsMeshComponent.hpp>
#include <GameFoundationLibrary/Components/PerspectiveCameraComponent.hpp>
#include <GameLogic/ComponentFactory.hpp>
#include <GameLogic/Entity.hpp>
#include "GameFoundationLibrary/Components/TransformComponent.hpp"


using namespace inl;
using namespace inl::game;


TestLevelSystem::TestLevelSystem(std::shared_ptr<const DynamicTuple> modules) : m_modules(modules) {}

void TestLevelSystem::ReactActions(ActionHeap& actions) {
	m_transientActionHeap = actions;
}


void TestLevelSystem::Modify(inl::game::Scene& scene) {

	struct : Visitor<LoadTestLevelAction> {
		void operator()(const LoadTestLevelAction& action) const {
			system.Load(scene, ComponentFactory_Singleton::GetInstance());
		}

		TestLevelSystem& system;
		Scene& scene;
	} visitor{ .system = *this, .scene = scene };

	m_transientActionHeap.value().get().Visit(visitor);
}


void TestLevelSystem::EmitActions(ActionHeap& actions) {
	m_transientActionHeap.reset();
}


void TestLevelSystem::Load(inl::game::Scene& scene, inl::game::ComponentFactory& factory) const {	
	using namespace inl::game;
	using namespace inl::gamelib;
	using namespace inl;

	const ComponentFactory& componentFactory = ComponentFactory_Singleton::GetInstance();
	GraphicsModule& graphicsModule = *m_modules->Get<const std::shared_ptr<GraphicsModule>&>();

	// Get scene.
	gxeng::IScene& graphicsScene = graphicsModule.GetOrCreateScene("MainScene");

	// Create a directional light.
	Entity& light = scene.CreateEntity();
	componentFactory.Create<DirectionalLightComponent>(light);
	auto&& lightComponent = light.GetFirstComponent<DirectionalLightComponent>();
	lightComponent.sceneName = "MainScene";
	lightComponent.entity = graphicsModule.CreateDirectionalLight();
	lightComponent.entity->SetDirection(Vec3{ 0.5, 0.5, -0.2 }.Normalized());
	lightComponent.entity->SetColor({ 1.0f, 0.7f, 0.4f });
	graphicsScene.GetEntities<gxeng::IDirectionalLight>().Add(lightComponent.entity.get());

	// Create terrain.
	//Entity& terrain = scene.CreateEntity();
	//componentFactory.Create<GraphicsMeshComponent>(terrain);
	//auto&& terrainComponent = terrain.GetFirstComponent<GraphicsMeshComponent>();
	//terrainComponent.sceneName = "MainScene";
	//terrainComponent.meshPath = "Models/Test/Terrain/terrain.fbx";
	//terrainComponent.materialPath = "Models/Test/Terrain/terrain.mtl";
	//terrainComponent.entity = graphicsModule.CreateMeshEntity();
	//terrainComponent.entity->SetMesh(graphicsModule.LoadMesh(terrainComponent.meshPath));
	//terrainComponent.entity->SetMaterial(graphicsModule.LoadMaterial(terrainComponent.materialPath));
	//graphicsScene.GetEntities<gxeng::IMeshEntity>().Add(terrainComponent.entity.get());

	//terrain.AddComponent(TransformComponent{});
	
	// Create a tree.
	Entity& tree = scene.CreateEntity();
	componentFactory.Create<GraphicsMeshComponent>(tree);
	auto&& treeComponent = tree.GetFirstComponent<GraphicsMeshComponent>();
	treeComponent.sceneName = "MainScene";
	treeComponent.meshPath = "Models/Vegetation/Trees/chestnut.fbx";
	treeComponent.materialPath = "Models/Vegetation/Trees/chestnut.mtl";
	treeComponent.entity = graphicsModule.CreateMeshEntity();
	treeComponent.entity->SetMesh(graphicsModule.LoadMesh(treeComponent.meshPath));
	treeComponent.entity->SetMaterial(graphicsModule.LoadMaterial(treeComponent.materialPath));
	graphicsScene.GetEntities<gxeng::IMeshEntity>().Add(treeComponent.entity.get());

	tree.AddComponent(TransformComponent{});
	tree.GetFirstComponent<TransformComponent>().SetScale({ 0.003937, 0.003937, 0.003937 });
	
	// Create a 3D camera.
	Entity& camera = scene.CreateEntity();
	componentFactory.Create<PerspectiveCameraComponent>(camera);
	auto&& cameraComponent = camera.GetFirstComponent<PerspectiveCameraComponent>();
	cameraComponent.entity = graphicsModule.CreatePerspectiveCamera("MainCamera");
	cameraComponent.entity->SetFOVAspect(Deg2Rad(75.f), 1.33f);
	cameraComponent.entity->SetNearPlane(0.5f);
	cameraComponent.entity->SetFarPlane(200.f);
	cameraComponent.entity->SetUpVector({ 0, 0, 1 });
	cameraComponent.entity->SetPosition({ 20.f, 5.f, 4.f });
	cameraComponent.entity->SetTarget({ 0, 0, 0 });

	// Create test heightmap entity.
	Entity& heightmap = scene.CreateEntity();
	componentFactory.Create<GraphicsHeightmapComponent>(heightmap);
	auto&& heightmapComponent = heightmap.GetFirstComponent<GraphicsHeightmapComponent>();
	heightmapComponent.sceneName = "MainScene";
	heightmapComponent.meshPath = "Models/Test/Heightmap/heightmap.fbx";
	heightmapComponent.materialPath = "Models/Test/Heightmap/heightmap.mtl";
	heightmapComponent.heightmapPath = "Models/Test/Heightmap/heightmap.png";
	heightmapComponent.entity = graphicsModule.CreateHeightmapEntity();
	heightmapComponent.entity->SetMesh(graphicsModule.LoadMesh(heightmapComponent.meshPath));
	heightmapComponent.entity->SetMaterial(graphicsModule.LoadMaterial(heightmapComponent.materialPath));
	heightmapComponent.entity->SetHeightmap(graphicsModule.LoadImage(heightmapComponent.heightmapPath));
	heightmapComponent.entity->SetUvSize({ 5, 5 });
	heightmapComponent.entity->SetMagnitude(0.3f);
	graphicsScene.GetEntities<gxeng::IHeightmapEntity>().Add(heightmapComponent.entity.get());
	
	heightmap.AddComponent(TransformComponent{});
	heightmap.GetFirstComponent<TransformComponent>().Move({ 0, 0, 1 });
	
	// Create heightmap terrain.
	Entity& hmTerrain = scene.CreateEntity();
	componentFactory.Create<GraphicsHeightmapComponent>(hmTerrain);
	auto&& hmTerrainComponent = hmTerrain.GetFirstComponent<GraphicsHeightmapComponent>();
	hmTerrainComponent.sceneName = "MainScene";
	hmTerrainComponent.meshPath = "Models/Test/Heightmap/heightmap.fbx";
	hmTerrainComponent.materialPath = "Models/Terrain/Helmfirth/terrain.mtl";
	hmTerrainComponent.heightmapPath = "Models/Terrain/Helmfirth/terrain.png";
	hmTerrainComponent.entity = graphicsModule.CreateHeightmapEntity();
	hmTerrainComponent.entity->SetMesh(graphicsModule.LoadMesh(hmTerrainComponent.meshPath));
	hmTerrainComponent.entity->SetMaterial(graphicsModule.LoadMaterial(hmTerrainComponent.materialPath));
	hmTerrainComponent.entity->SetHeightmap(graphicsModule.LoadImage(hmTerrainComponent.heightmapPath));
	hmTerrainComponent.entity->SetUvSize({ 5, 5 });
	hmTerrainComponent.entity->SetMagnitude(25.5f);
	hmTerrainComponent.entity->SetOffset(-12.7f);
	graphicsScene.GetEntities<gxeng::IHeightmapEntity>().Add(hmTerrainComponent.entity.get());

	TransformComponent hmTerrainTransform;
	hmTerrainTransform.SetScale({ 10, 10, 10 });
	hmTerrain.AddComponent(hmTerrainTransform);
}
