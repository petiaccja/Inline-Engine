#include "TestLevel.hpp"

#include "GameFoundationLibrary/Components/DirectionalLightComponent.hpp"


inl::game::Scene TestLevel::Initialize(inl::game::ComponentFactory& componentFactory, inl::Vec3 centerPosition) {
	using namespace inl;
	using namespace game;
	using namespace gamelib;

	Scene newWorld;

	// Create a directional light.
	Entity* entity = newWorld.CreateEntity();
	componentFactory.Create<DirectionalLightComponent>(*entity);
	auto&& dcl = entity->GetFirstComponent<DirectionalLightComponent>().entity;
	dcl->SetDirection(Vec3{ 0.5, 0.5, -0.3 }.Normalized());
	dcl->SetColor({ 1.0f, 0.9f, 0.8f });

	//auto& directionalLightFactory = componentFactory.GetClassFactory<DirectionalLightComponent>();
	//auto mainSceneIt = directionalLightFactory.GetScenes().find("MainScene");
	//if (mainSceneIt != directionalLightFactory.GetScenes().end()) {
	//	mainSceneIt->second->GetEntities<gxeng::IDirectionalLight>().Add(dcl.get());
	//}

	return newWorld;
}

const std::string& TestLevel::GetName() const {
	static const std::string name = "Test level";
	return name;
}
