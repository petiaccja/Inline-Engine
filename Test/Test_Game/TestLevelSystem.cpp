#include "TestLevelSystem.hpp"

#include <BaseLibrary/AtScopeExit.hpp>
#include <BaseLibrary/DynamicTuple.hpp>
#include <GameFoundationLibrary/Components/DirectionalLightComponent.hpp>
#include <GameFoundationLibrary/Components/GraphicsHeightmapComponent.hpp>
#include <GameFoundationLibrary/Components/GraphicsMeshComponent.hpp>
#include <GameFoundationLibrary/Components/PerspectiveCameraComponent.hpp>
#include <GameLogic/ComponentFactory.hpp>
#include <GameLogic/Entity.hpp>

void TestLevelSystem::Update(float elapsed) {
	// Do nothing.
}

void TestLevelSystem::Create(const CreateEntity& createEntity) {
	using namespace inl::game;
	using namespace inl::gamelib;
	using namespace inl;

	if (!m_command) {
		return;
	}
	AtScopeExit ase{ [this] { m_command.reset(); } };

	const ComponentFactory& componentFactory = m_command->componentFactory;
	GraphicsModule& graphicsModule = *m_command->modules.Get<GraphicsModule*>();

	// Get scene.
	gxeng::IScene& scene = graphicsModule.GetOrCreateScene("MainScene");

	// Create a directional light.
	Entity& light = createEntity();
	componentFactory.Create<DirectionalLightComponent>(light);
	auto&& lightComponent = light.GetFirstComponent<DirectionalLightComponent>();
	lightComponent.entity = graphicsModule.CreateDirectionalLight();
	lightComponent.entity->SetDirection(Vec3{ 0.5, 0.5, -0.2 }.Normalized());
	lightComponent.entity->SetColor({ 1.0f, 0.7f, 0.4f });
	scene.GetEntities<gxeng::IDirectionalLight>().Add(lightComponent.entity.get());

	// Create terrain.
	Entity& terrain = createEntity();
	componentFactory.Create<GraphicsMeshComponent>(terrain);
	auto&& terrainComponent = terrain.GetFirstComponent<GraphicsMeshComponent>();
	terrainComponent.entity = graphicsModule.CreateMeshEntity();
	terrainComponent.mesh = graphicsModule.LoadMesh("Models/Terrain/terrain.fbx");
	terrainComponent.material = graphicsModule.LoadMaterial("Models/Terrain/terrain.mtl");
	terrainComponent.entity->SetMesh(terrainComponent.mesh.get());
	terrainComponent.entity->SetMaterial(terrainComponent.material.get());
	terrainComponent.entity->SetPosition({ 0, 0, 0 });
	terrainComponent.entity->SetRotation(Quat::Identity());
	terrainComponent.entity->SetScale({ 1, 1, 1 });
	scene.GetEntities<gxeng::IMeshEntity>().Add(terrainComponent.entity.get());

	// Create a tree.
	Entity& tree = createEntity();
	componentFactory.Create<GraphicsMeshComponent>(tree);
	auto&& treeComponent = tree.GetFirstComponent<GraphicsMeshComponent>();
	treeComponent.entity = graphicsModule.CreateMeshEntity();
	treeComponent.mesh = graphicsModule.LoadMesh("Models/Vegetation/Trees/chestnut.fbx");
	treeComponent.material = graphicsModule.LoadMaterial("Models/Vegetation/Trees/chestnut.mtl");
	treeComponent.entity->SetMesh(treeComponent.mesh.get());
	treeComponent.entity->SetMaterial(treeComponent.material.get());
	treeComponent.entity->SetPosition({ 0, 0, 0 });
	treeComponent.entity->SetRotation(Quat::Identity());
	treeComponent.entity->SetScale({ 0.003937, 0.003937, 0.003937 });
	scene.GetEntities<gxeng::IMeshEntity>().Add(treeComponent.entity.get());

	// Create a 3D camera.
	Entity& camera = createEntity();
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
	Entity& heightmap = createEntity();
	componentFactory.Create<GraphicsHeightmapComponent>(heightmap);
	auto&& heightmapComponent = heightmap.GetFirstComponent<GraphicsHeightmapComponent>();
	heightmapComponent.entity = graphicsModule.CreateHeightmapEntity();
	heightmapComponent.mesh = graphicsModule.LoadMesh("Models/Test/Heightmap/heightmap.fbx");
	heightmapComponent.material = graphicsModule.LoadMaterial("Models/Test/Heightmap/heightmap.mtl");
	heightmapComponent.heightmap = graphicsModule.LoadImage("Models/Test/Heightmap/heightmap.png");
	heightmapComponent.entity->SetMesh(heightmapComponent.mesh.get());
	heightmapComponent.entity->SetMaterial(heightmapComponent.material.get());
	heightmapComponent.entity->SetHeightmap(heightmapComponent.heightmap.get());
	heightmapComponent.entity->SetPosition({ 0, 0, 1 });
	heightmapComponent.entity->SetRotation(Quat::Identity());
	heightmapComponent.entity->SetScale({ 1, 1, 1 });
	heightmapComponent.entity->SetUvSize({ 5, 5 });
	heightmapComponent.entity->SetMagnitude(0.3f);
	scene.GetEntities<gxeng::IHeightmapEntity>().Add(heightmapComponent.entity.get());
}

void TestLevelSystem::LoadAsync(const inl::game::ComponentFactory& componentFactory, const inl::DynamicTuple& subsystems) {
	m_command.emplace(Command{ componentFactory, subsystems });
}
