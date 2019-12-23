#include "Systems.hpp"

#include <GameLogic/System.hpp>

#include <Catch2/catch.hpp>

using namespace inl::game;

#pragma message("Rewrite all these tests")

TEST_CASE("System - Update default", "[GameLogic:System]") {
	DoubleFooToBarSystem system;

	ComponentMatrix store;
	store.types.push_back(_ComponentVector<FooComponent>{});
	store.types.push_back(_ComponentVector<BarComponent>{});
	store.types.push_back(_ComponentVector<BazComponent>{});

	store.entities.emplace_back(FooComponent{ 1 }, BarComponent{ 4 }, BazComponent{ 7 });
	store.entities.emplace_back(FooComponent{ 2 }, BarComponent{ 5 }, BazComponent{ 8 });
	store.entities.emplace_back(FooComponent{ 3 }, BarComponent{ 6 }, BazComponent{ 9 });

	ComponentRange<const FooComponent, BarComponent> range(store);

	//system.Update(0.0f, store);

	for (auto [foo, bar] : range) {
		REQUIRE(foo.value * 2 == bar.value);
	}
}


TEST_CASE("System - Update no components", "[GameLogic:System]") {
	StandaloneSystem system;

	//system.Update(0.0f);

	REQUIRE(system.content == "use renewables;");
}