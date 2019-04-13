#include "Systems.hpp"

#include <GameLogic/System.hpp>

#include <Catch2/catch.hpp>

using namespace inl::game;


TEST_CASE("System - Update default", "[GameLogic]") {
	DoubleFooToBarSystem system;

	ComponentStore store;
	store.Extend<FooComponent>();
	store.Extend<BarComponent>();
	store.Extend<BazComponent>();

	store.PushBack(FooComponent{ 1 }, BarComponent{ 4 }, BazComponent{ 7 });
	store.PushBack(FooComponent{ 2 }, BarComponent{ 5 }, BazComponent{ 8 });
	store.PushBack(FooComponent{ 3 }, BarComponent{ 6 }, BazComponent{ 9 });

	ComponentRange<const FooComponent, BarComponent> range(store);

	system.Update(store);

	for (auto [foo, bar] : range) {
		REQUIRE(foo.value * 2 == bar.value);
	}
}


TEST_CASE("System - Update no components", "[GameLogic]") {
	StandaloneSystem system;

	system.Update();

	REQUIRE(system.content == "use renewables;");
}