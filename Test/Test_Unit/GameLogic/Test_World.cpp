#include "Components.hpp"

#include <GameLogic/World.hpp>

#include <Catch2/catch.hpp>

using namespace inl::game;


TEST_CASE("World - AddEntity", "[GameLogic]") {
	World world;
	auto entity = world.CreateEntity(FooComponent{}, BarComponent{}, BazComponent{});

	REQUIRE(entity->GetStore()->store.entities.size() == 1);
	REQUIRE(entity->GetIndex() == 0);
	REQUIRE(entity->GetWorld() == &world);
}


TEST_CASE("World - Delete entity", "[GameLogic]") {
	World world;
	auto entity1 = world.CreateEntity(FooComponent{}, BarComponent{}, BazComponent{});
	auto entity2 = world.CreateEntity(FooComponent{}, BarComponent{}, BazComponent{});

	world.DeleteEntity(*entity1);

	REQUIRE(entity2->GetStore()->store.entities.size() == 1);
	REQUIRE(entity2->GetIndex() == 0);
	REQUIRE(entity2->GetWorld() == &world);
}



TEST_CASE("World - Add multiple entities", "[GameLogic]") {
	World world;
	auto entity0 = world.CreateEntity(FooComponent{}, BarComponent{}, BazComponent{});
	auto entity1 = world.CreateEntity(FooComponent{}, BarComponent{}, BazComponent{});

	REQUIRE(entity0->GetStore()->store.entities.size() == 2);
	REQUIRE(entity0->GetIndex() == 0);
	REQUIRE(entity0->GetWorld() == &world);

	REQUIRE(entity1->GetStore()->store.entities.size() == 2);
	REQUIRE(entity1->GetIndex() == 1);
	REQUIRE(entity1->GetWorld() == &world);
}


TEST_CASE("World - Add different entities", "[GameLogic]") {
	World world;
	auto entity0 = world.CreateEntity(FooComponent{}, BarComponent{}, BazComponent{});
	auto entity1 = world.CreateEntity(FooComponent{}, BarComponent{});

	REQUIRE(entity0->GetStore()->store.entities.size() == 1);
	REQUIRE(entity0->GetIndex() == 0);
	REQUIRE(entity0->GetWorld() == &world);

	REQUIRE(entity1->GetStore()->store.entities.size() == 1);
	REQUIRE(entity1->GetIndex() == 0);
	REQUIRE(entity1->GetWorld() == &world);
}


TEST_CASE("World - Add component new", "[GameLogic]") {
	World world;
	auto entity0 = world.CreateEntity(FooComponent{}, BarComponent{}, BazComponent{});
	auto entity1 = world.CreateEntity(FooComponent{}, BarComponent{});
	world.AddComponent(*entity0, BazComponent{});

	REQUIRE(entity0->GetStore()->store.entities.size() == 1);
	REQUIRE(entity0->GetStore()->store.types.size() == 4);
	REQUIRE(entity0->GetIndex() == 0);
	REQUIRE(entity0->GetWorld() == &world);

	REQUIRE(entity1->GetStore()->store.entities.size() == 1);
	REQUIRE(entity1->GetStore()->store.types.size() == 2);
	REQUIRE(entity1->GetIndex() == 0);
	REQUIRE(entity1->GetWorld() == &world);
}


TEST_CASE("World - Add component merge", "[GameLogic]") {
	World world;
	auto entity0 = world.CreateEntity(FooComponent{}, BarComponent{}, BazComponent{});
	auto entity1 = world.CreateEntity(FooComponent{}, BarComponent{});
	world.AddComponent(*entity1, BazComponent{});

	REQUIRE(entity0->GetStore()->store.entities.size() == 2);
	REQUIRE(entity0->GetIndex() == 0);
	REQUIRE(entity0->GetWorld() == &world);

	REQUIRE(entity1->GetStore() == entity0->GetStore());
	REQUIRE(entity1->GetIndex() == 1);
	REQUIRE(entity1->GetWorld() == &world);
}


TEST_CASE("World - Remove component new", "[GameLogic]") {
	World world;
	auto entity0 = world.CreateEntity(FooComponent{}, BarComponent{}, BazComponent{});
	auto entity1 = world.CreateEntity(FooComponent{}, BarComponent{});
	world.RemoveComponent<BarComponent>(*entity1);

	REQUIRE(entity0->GetStore()->store.entities.size() == 1);
	REQUIRE(entity0->GetStore()->store.types.size() == 3);
	REQUIRE(entity0->GetIndex() == 0);
	REQUIRE(entity0->GetWorld() == &world);

	REQUIRE(entity1->GetStore()->store.entities.size() == 1);
	REQUIRE(entity1->GetStore()->store.types.size() == 1);
	REQUIRE(entity1->GetIndex() == 0);
	REQUIRE(entity1->GetWorld() == &world);
}


TEST_CASE("World - Remove component merge", "[GameLogic]") {
	World world;
	auto entity0 = world.CreateEntity(FooComponent{}, BarComponent{}, BazComponent{});
	auto entity1 = world.CreateEntity(FooComponent{}, BarComponent{});
	world.RemoveComponent<BazComponent>(*entity0);

	REQUIRE(entity0->GetStore()->store.entities.size() == 2);
	REQUIRE(entity0->GetStore()->store.types.size() == 2);
	REQUIRE(entity0->GetIndex() == 1);
	REQUIRE(entity0->GetWorld() == &world);

	REQUIRE(entity1->GetStore() == entity0->GetStore());
	REQUIRE(entity1->GetIndex() == 0);
	REQUIRE(entity1->GetWorld() == &world);
}


TEST_CASE("World - Create empty entities", "[GameLogic]") {
	World world;
	auto entity0 = world.CreateEntity();
	world.AddComponent(*entity0, BazComponent{});
	auto entity1 = world.CreateEntity();
	world.AddComponent(*entity1, BarComponent{});

	REQUIRE(entity0->HasComponent<BazComponent>());
	REQUIRE(entity1->HasComponent<BarComponent>());
}
