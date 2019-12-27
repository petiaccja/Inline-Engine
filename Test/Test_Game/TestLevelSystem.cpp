#include "TestLevelSystem.hpp"

#include <BaseLibrary/AtScopeExit.hpp>
#include <BaseLibrary/DynamicTuple.hpp>
#include <GameFoundationLibrary/Components/DirectionalLightComponent.hpp>
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
	lightComponent.entity->SetDirection(Vec3{ 0.5, 0.5, -0.3 }.Normalized());
	lightComponent.entity->SetColor({ 1.0f, 0.9f, 0.8f });
	scene.GetEntities<gxeng::IDirectionalLight>().Add(lightComponent.entity.get());

	// Create a 3D camera.
	Entity& camera = createEntity();
	componentFactory.Create<PerspectiveCameraComponent>(camera);
	auto&& cameraComponent = camera.GetFirstComponent<PerspectiveCameraComponent>();
	cameraComponent.entity = graphicsModule.CreatePerspectiveCamera("MainCamera");
	cameraComponent.entity->SetFOVAspect(75.f, 1.33f);
	cameraComponent.entity->SetNearPlane(0.5f);
	cameraComponent.entity->SetFarPlane(200.f);
	cameraComponent.entity->SetUpVector({ 0, 0, 1 });
	cameraComponent.entity->SetPosition({ 5.f, 5.f, 2.f });
	cameraComponent.entity->SetTarget({ 0, 0, 0 });
}

void TestLevelSystem::LoadAsync(const inl::game::ComponentFactory& componentFactory, const inl::DynamicTuple& subsystems) {
	m_command.emplace(Command{ componentFactory, subsystems });
}
