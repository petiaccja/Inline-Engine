#include "Systems.hpp"

#include <GameLogic/System.hpp>

#include <Catch2/catch.hpp>

using namespace inl::game;


TEST_CASE("System - Update default", "[GameLogic:System]") {
	DoubleFooToBarSystem system;

	Scene scene;
	EntitySchemeSet set(scene);
	set.SetComponentTypes<FooComponent, BarComponent, BazComponent>();


	set.Create(FooComponent{ 1 }, BarComponent{ 4 }, BazComponent{ 7 });
	set.Create(FooComponent{ 2 }, BarComponent{ 5 }, BazComponent{ 8 });
	set.Create(FooComponent{ 3 }, BarComponent{ 6 }, BazComponent{ 9 });

	ComponentRange<const FooComponent, BarComponent> range(set.GetMatrix());

	system.Run(0.0f, set, scene);

	for (auto [foo, bar] : range) {
		REQUIRE(foo.value * 2 == bar.value);
	}
}


TEST_CASE("System - Update no components", "[GameLogic:System]") {
	StandaloneSystem system;

	Scene scene;
	system.Run(0.0f, scene);

	REQUIRE(system.content == "use renewables;");
}