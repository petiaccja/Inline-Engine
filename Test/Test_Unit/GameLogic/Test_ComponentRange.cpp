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


TEST_CASE("ComponentRange - Iterator", "[GameLogic]") {
	ComponentStore store;
	store.Extend<FooComponent>();
	store.Extend<BarComponent>();
	store.Extend<BazComponent>();

	store.PushBack(FooComponent{ 1 }, BarComponent{ 4 }, BazComponent{ 7 });
	store.PushBack(FooComponent{ 2 }, BarComponent{ 5 }, BazComponent{ 8 });
	store.PushBack(FooComponent{ 3 }, BarComponent{ 6 }, BazComponent{ 9 });

	ComponentRange<const FooComponent, BazComponent> range(store);

	auto beg = range.begin();
	auto end = range.end();

	REQUIRE(std::distance(beg, end) == 3);

	int expectedValue = 0;
	for (auto it = beg; it != end; ++it, ++expectedValue) {
		REQUIRE(std::get<0>(*it).value == expectedValue + 1);
		REQUIRE(std::get<1>(*it).value == expectedValue + 7);
	}
}


TEST_CASE("ComponentRange - Modify elements", "[GameLogic]") {
	ComponentStore store;
	store.Extend<FooComponent>();
	store.Extend<BarComponent>();
	store.Extend<BazComponent>();

	store.PushBack(FooComponent{ 1 }, BarComponent{ 4 }, BazComponent{ 7 });
	store.PushBack(FooComponent{ 2 }, BarComponent{ 5 }, BazComponent{ 8 });
	store.PushBack(FooComponent{ 3 }, BarComponent{ 6 }, BazComponent{ 9 });

	ComponentRange<const FooComponent, BazComponent> range(store);

	for (auto [foo, baz] : range) {
		REQUIRE(baz.value != foo.value);
	}
	for (auto [foo, baz] : range) {
		baz.value = foo.value;
	}
	for (auto [foo, baz] : range) {
		REQUIRE(baz.value == foo.value);
	}
}