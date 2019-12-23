#include "Components.hpp"

#include <GameLogic/ContiguousVector.hpp>

#include <Catch2/catch.hpp>

using namespace inl::game;


TEST_CASE("Erase single", "[GameLogic:ContiguousVector]") {
	ContiguousVector<int> v = { 0, 1, 2, 10, 4, 5, 3 };
	v.erase(v.begin() + 3);

	REQUIRE(v.size() == 6);
	for (size_t i = 0; i < v.size(); ++i) {
		REQUIRE(v[i] == i);
	}
}


TEST_CASE("Erase range extra", "[GameLogic:ContiguousVector]") {
	ContiguousVector<int> v = { 0, 1, 2, 10, 11, 12, 6, 3, 4, 5 };
	v.erase(v.begin() + 3, v.begin() + 6);

	REQUIRE(v.size() == 7);
	for (size_t i = 0; i < v.size(); ++i) {
		REQUIRE(v[i] == i);
	}
}

TEST_CASE("Erase range optimized", "[GameLogic:ContiguousVector]") {
	ContiguousVector<int> v = { 0, 1, 2, 10, 11, 12, 3, 4 };
	v.erase(v.begin() + 3, v.begin() + 6);

	REQUIRE(v.size() == 5);
	for (size_t i = 0; i < v.size(); ++i) {
		REQUIRE(v[i] == i);
	}
}


TEST_CASE("Swap elems", "[GameLogic:ContiguousVector]") {
	ContiguousVector<int> v = { 0, 1, 3, 2, 4, 5 };
	v.swap_elements(v.begin() + 2, v.begin() + 3);

	REQUIRE(v.size() == 6);
	for (size_t i = 0; i < v.size(); ++i) {
		REQUIRE(v[i] == i);
	}	
}