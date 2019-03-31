#include "Components.hpp"

#include <GameLogic/ComponentScheme.hpp>

#include <Catch2/catch.hpp>
#include <algorithm>

using namespace inl::game;



TEST_CASE("ComponentScheme - Ctor empty", "[GameLogic]") {
	ComponentScheme scheme;
	REQUIRE(scheme.Size() == 0);
}


TEST_CASE("ComponentScheme - Ctor list", "[GameLogic]") {
	ComponentScheme scheme = { typeid(FooComponent), typeid(BarComponent), typeid(BarComponent) };
	REQUIRE(scheme.Size() == 3);
	REQUIRE(std::is_sorted(scheme.begin(), scheme.end()));
}


TEST_CASE("ComponentScheme - Iterators & Size", "[GameLogic]") {
	ComponentScheme scheme = { typeid(FooComponent), typeid(BarComponent), typeid(BarComponent) };
	REQUIRE(scheme.Size() == 3);
	REQUIRE(std::distance(scheme.begin(), scheme.end()) == 3);
}


TEST_CASE("ComponentScheme - Insert", "[GameLogic]") {
	ComponentScheme scheme;
	scheme.Insert(typeid(FooComponent));
	scheme.Insert(typeid(BarComponent));
	scheme.Insert(typeid(FooComponent));
	scheme.Insert(typeid(BazComponent));
	scheme.Insert(typeid(FooComponent));
	REQUIRE(scheme.Size() == 5);
	REQUIRE(std::is_sorted(scheme.begin(), scheme.end()));
}


TEST_CASE("ComponentScheme - Search", "[GameLogic]") {
	ComponentScheme scheme = {
		typeid(FooComponent),
		typeid(BarComponent),
		typeid(FooComponent),
		typeid(BazComponent),
		typeid(FooComponent),
	};

	auto [first1, last1] = scheme.Range(typeid(BarComponent));
	auto [first2, last2] = scheme.Range(typeid(FooComponent));
	auto [first3, last3] = scheme.Range(typeid(BazComponent));
	REQUIRE(std::distance(first1, last1) == 1);
	REQUIRE(std::distance(first2, last2) == 3);
	REQUIRE(std::distance(first3, last3) == 1);
	for (; first1 != last1; ++first1)
		REQUIRE(*first1 == typeid(BarComponent));
	for (; first2 != last2; ++first2)
		REQUIRE(*first2 == typeid(FooComponent));
	for (; first3 != last3; ++first3)
		REQUIRE(*first3 == typeid(BazComponent));
}


TEST_CASE("ComponentScheme - Erase", "[GameLogic]") {
	ComponentScheme scheme = {
		typeid(FooComponent),
		typeid(BarComponent),
		typeid(FooComponent),
		typeid(BazComponent),
		typeid(FooComponent),
	};

	auto [first, last] = scheme.Range(typeid(BazComponent));
	scheme.Erase(first);

	REQUIRE(scheme.Size() == 4);
	REQUIRE(std::is_sorted(scheme.begin(), scheme.end()));
	auto [firstPost, lastPost] = scheme.Range(typeid(BazComponent));
	REQUIRE(std::distance(firstPost, lastPost) == 0);
}


TEST_CASE("ComponentScheme - Comparison", "[GameLogic]") {
	ComponentScheme scheme1 = {
		typeid(FooComponent),
		typeid(BarComponent),
		typeid(FooComponent),
		typeid(BazComponent),
		typeid(FooComponent),
	};

	ComponentScheme scheme2 = {
		typeid(FooComponent),
		typeid(BarComponent),
		typeid(FooComponent),
		typeid(BazComponent),
		typeid(FooComponent),
	};

	ComponentScheme scheme3 = {
		typeid(FooComponent),
		typeid(BarComponent),
		typeid(FooComponent),
		typeid(FooComponent),
	};

	REQUIRE(scheme1 == scheme2);
	REQUIRE(!(scheme1 != scheme2));
	REQUIRE(scheme1 != scheme3);
	REQUIRE(!(scheme1 == scheme3));
}


TEST_CASE("ComponentScheme - Hash equality", "[GameLogic]") {
	ComponentScheme scheme1 = {
		typeid(FooComponent),
		typeid(BarComponent),
		typeid(FooComponent),
		typeid(BazComponent),
		typeid(FooComponent),
	};

	ComponentScheme scheme3 = {
		typeid(FooComponent),
		typeid(BarComponent),
		typeid(FooComponent),
		typeid(FooComponent),
	};

	REQUIRE(scheme1.GetHashCode() != scheme3.GetHashCode());
}
