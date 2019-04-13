#include "Components.hpp"

#include <GameLogic/ComponentStore.hpp>

#include <Catch2/catch.hpp>

using namespace inl::game;


TEST_CASE("ComponentStore - Default", "[GameLogic]") {
	ComponentStore store;
	REQUIRE(store.Scheme().Size() == 0);
	REQUIRE(store.Size() == 0);
}


TEST_CASE("ComponentStore - Extend", "[GameLogic]") {
	ComponentStore store;
	store.Extend<FooComponent>();
	store.Extend<BarComponent>();
	store.Extend<BazComponent>();

	ComponentScheme expectedScheme = {
		typeid(FooComponent),
		typeid(BarComponent),
		typeid(BazComponent),
	};

	REQUIRE(store.Scheme() == expectedScheme);
	REQUIRE(store.Size() == 0);
}


TEST_CASE("ComponentStore - Push back", "[GameLogic]") {
	ComponentStore store;
	store.Extend<FooComponent>();
	store.Extend<BarComponent>();
	store.Extend<BazComponent>();

	store.PushBack(FooComponent{}, BarComponent{}, BazComponent{});
	store.PushBack(FooComponent{}, BazComponent{}, BarComponent{});
	store.PushBack(BarComponent{}, FooComponent{}, BazComponent{});

	REQUIRE(store.Size() == 3);
}


TEST_CASE("ComponentStore - Splice back superset", "[GameLogic]") {
	ComponentStore targetStore;
	targetStore.Extend<FooComponent>();
	targetStore.Extend<FooComponent>();
	targetStore.Extend<BarComponent>();
	targetStore.Extend<BazComponent>();

	ComponentStore sourceStore;
	sourceStore.Extend<FooComponent>();
	sourceStore.Extend<BarComponent>();
	sourceStore.Extend<BazComponent>();

	sourceStore.PushBack(FooComponent{}, BarComponent{}, BazComponent{});
	sourceStore.PushBack(FooComponent{}, BazComponent{}, BarComponent{});
	REQUIRE(targetStore.Size() == 0);
	REQUIRE(sourceStore.Size() == 2);

	targetStore.SpliceBackExtend(sourceStore, 1, FooComponent{ 12.f });

	REQUIRE(targetStore.Size() == 1);
	REQUIRE(sourceStore.Size() == 1);
}



TEST_CASE("ComponentStore - Splice back subset", "[GameLogic]") {
	ComponentStore targetStore;
	targetStore.Extend<FooComponent>();
	targetStore.Extend<BarComponent>();
	targetStore.Extend<BazComponent>();

	ComponentStore sourceStore;
	sourceStore.Extend<FooComponent>();
	sourceStore.Extend<FooComponent>();
	sourceStore.Extend<BarComponent>();
	sourceStore.Extend<BazComponent>();

	sourceStore.PushBack(FooComponent{10.f}, FooComponent{20.f}, BarComponent{}, BazComponent{});
	sourceStore.PushBack(FooComponent{10.f}, FooComponent{20.f}, BazComponent{}, BarComponent{});
	REQUIRE(targetStore.Size() == 0);
	REQUIRE(sourceStore.Size() == 2);

	std::vector<bool> selection(sourceStore.Scheme().Size(), true);
	auto scheme = sourceStore.Scheme();
	selection[scheme.Index(typeid(FooComponent)).first] = false;
	targetStore.SpliceBackReduce(sourceStore, 1, selection);

	REQUIRE(targetStore.Size() == 1);
	REQUIRE(sourceStore.Size() == 1);

	auto sourceScheme = sourceStore.Scheme();
	auto& sourceVector = sourceStore.GetComponentVector<FooComponent>(sourceScheme.Index(typeid(FooComponent)).first);
	REQUIRE(sourceVector[0].value == 10.f);

	auto targetScheme = targetStore.Scheme();
	auto& targetVector = targetStore.GetComponentVector<FooComponent>(targetScheme.Index(typeid(FooComponent)).first);
	REQUIRE(targetVector[0].value == 20.f);
}