#include "Components.hpp"

#include <GameLogic/ComponentFactory.hpp>

#include <Catch2/catch.hpp>

using namespace inl::game;


TEST_CASE("ComponentFactory - Registration", "[GameLogic]") {
	REQUIRE(ComponentFactory_Singleton::GetInstance().IsRegistered<FooComponent>());
	REQUIRE(ComponentFactory_Singleton::GetInstance().IsRegistered<BarComponent>());
	REQUIRE(ComponentFactory_Singleton::GetInstance().IsRegistered<BazComponent>());
}


TEST_CASE("ComponentFactory - Create", "[GameLogic]") {
	World world;
	Entity& entity = *world.CreateEntity();
	ComponentFactory_Singleton::GetInstance().Create(entity, "FooComponent");
	REQUIRE(entity.HasComponent<FooComponent>());
}


TEST_CASE("ComponentFactory - Create with special factory", "[GameLogic]") {
	World world;
	Entity& entity = *world.CreateEntity();
	auto& factory = ComponentFactory_Singleton::GetInstance();
	auto& specialFactory = factory.GetClassFactory<SpecialComponent, SpecialFactory>();
	specialFactory.Configure(16.0f);
	factory.Create(entity, "SpecialComponent");

	REQUIRE(entity.HasComponent<SpecialComponent>());
	REQUIRE(entity.GetFirstComponent<SpecialComponent>().value == 16.0f);
}
