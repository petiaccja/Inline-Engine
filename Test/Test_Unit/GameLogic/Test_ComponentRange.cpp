#include "Components.hpp"

#include <GameLogic/ComponentRange.hpp>

#include <Catch2/catch.hpp>

using namespace inl::game;



TEST_CASE("ComponentRange - Construct", "[GameLogic:ComponentRange]") {
	ComponentMatrix store;
	store.types.push_back(ComponentVector<FooComponent>{});
	store.types.push_back(ComponentVector<BarComponent>{});
	store.types.push_back(ComponentVector<BazComponent>{});

	ComponentRange<const FooComponent, BazComponent> range(store);
}


TEST_CASE("ComponentRange - Iterator", "[GameLogic:ComponentRange]") {
	ComponentMatrix store;
	store.types.push_back(ComponentVector<FooComponent>{});
	store.types.push_back(ComponentVector<BarComponent>{});
	store.types.push_back(ComponentVector<BazComponent>{});

	store.entities.emplace_back(FooComponent{ 1 }, BarComponent{ 4 }, BazComponent{ 7 });
	store.entities.emplace_back(FooComponent{ 2 }, BarComponent{ 5 }, BazComponent{ 8 });
	store.entities.emplace_back(FooComponent{ 3 }, BarComponent{ 6 }, BazComponent{ 9 });

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


TEST_CASE("ComponentRange - Modify elements", "[GameLogic:ComponentRange]") {
	ComponentMatrix store;
	store.types.push_back(ComponentVector<FooComponent>{});
	store.types.push_back(ComponentVector<BarComponent>{});
	store.types.push_back(ComponentVector<BazComponent>{});

	store.entities.emplace_back(FooComponent{ 1 }, BarComponent{ 4 }, BazComponent{ 7 });
	store.entities.emplace_back(FooComponent{ 2 }, BarComponent{ 5 }, BazComponent{ 8 });
	store.entities.emplace_back(FooComponent{ 3 }, BarComponent{ 6 }, BazComponent{ 9 });

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


TEST_CASE("ComponentRange - Failure to get vectors", "[GameLogic:ComponentRange]") {
	ComponentMatrix store;
	store.types.push_back(ComponentVector<FooComponent>{});
	store.types.push_back(ComponentVector<BarComponent>{});

	store.entities.emplace_back(FooComponent{ 1 }, BarComponent{ 4 });
	store.entities.emplace_back(FooComponent{ 2 }, BarComponent{ 5 });
	store.entities.emplace_back(FooComponent{ 3 }, BarComponent{ 6 });

	REQUIRE_THROWS(ComponentRange<const FooComponent, BazComponent>(store));
}