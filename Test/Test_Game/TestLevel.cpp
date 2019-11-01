#include "TestLevel.hpp"

#include "GameFoundationLibrary/Components/DirectionalLightComponent.hpp"


inl::game::World TestLevel::Initialize(inl::game::ComponentFactory& componentFactory, inl::Vec3 centerPosition) {
	using namespace inl;
	using namespace game;
	using namespace gamelib;

	World newWorld;

	Entity* entity = newWorld.CreateEntity();
	componentFactory.Create<DirectionalLightComponent>(*entity);
	auto&& dcl = entity->GetFirstComponent<DirectionalLightComponent>().entity;
	dcl->SetDirection(Vec3{ 0.5, 0.5, -0.3 }.Normalized());
	dcl->SetColor({ 1.0f, 0.9f, 0.8f });

	return newWorld;
}
