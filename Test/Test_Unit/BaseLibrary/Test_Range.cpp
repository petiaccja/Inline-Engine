#include <BaseLibrary/Range.hpp>

#include <Catch2/catch.hpp>


using inl::Range;


TEST_CASE("Simple loop", "[Range]") {
	std::vector<int> v = { 1, 2, 3, 4, 5 };
	std::vector<int> u;
	for (auto i : Range(1, 6)) {
		u.push_back(i);
	}
	REQUIRE(u == v);
}


TEST_CASE("Bigger step aligned", "[Range]") {
	std::vector<int> v = { 3, 8, 13, 18, 23 };
	std::vector<int> u;
	for (auto i : Range(3u, 28u, 5u)) {
		u.push_back(i);
	}
	REQUIRE(u == v);
}


TEST_CASE("Bigger step unaligned", "[Range]") {
	std::vector<int> v = { 3, 8, 13, 18, 23 };
	std::vector<int> u;
	for (auto i : Range(3, 24, 5)) {
		u.push_back(i);
	}
	REQUIRE(u == v);
}


TEST_CASE("Simple loop negative", "[Range]") {
	std::vector<int> v = { 2, 1, 0, -1, -2 };
	std::vector<int> u;
	for (auto i : Range(2, -3)) {
		u.push_back(i);
	}
	REQUIRE(u == v);
}

TEST_CASE("Bigger step negative aligned", "[Range]") {
	std::vector<int> v = { 7, 2, -3, -8, -13 };
	std::vector<int> u;
	for (auto i : Range(7, -18, -5)) {
		u.push_back(i);
	}
	REQUIRE(u == v);
}

TEST_CASE("Bigger step negative unaligned", "[Range]") {
	std::vector<int> v = { 7, 2, -3, -8, -13 };
	std::vector<int> u;
	for (auto i : Range(7, -17, -5)) {
		u.push_back(i);
	}
	REQUIRE(u == v);
}