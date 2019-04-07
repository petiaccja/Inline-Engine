#include "Components.hpp"

#include <GameLogic/ComponentRange.hpp>

#include <Catch2/catch.hpp>

using namespace inl::game;



TEST_CASE("ComponentRange - Construct", "[GameLogic]") {
	ComponentStore store;
	store.Extend<FooComponent>();
	store.Extend<BarComponent>();
	store.Extend<BazComponent>();

	ComponentRange<const FooComponent, BazComponent> range(store);

	
}