#include <BaseLibrary/Color.hpp>

#include <Catch2/catch.hpp>
#include <cstdint>

using namespace inl;


TEST_CASE("Float -> float", "[Color]") {
	Color<float> c1, c2;
	c1 = { 0.5, 0.5, 0.5, 0.5 };
	c2 = c1;
	REQUIRE(c1 == c2);
}


TEST_CASE("Float -> uint16_t", "[Color]") {
	Color<float> c1(1, 0.5f, 0, 1);
	Color<uint16_t> c2(c1);
	REQUIRE(c2 == Color<uint16_t>(65535, 32768, 0, 65535));
}


TEST_CASE("Float -> int16_t", "[Color]") {
	Color<float> c1(1, 0.5f, 0, 1);
	Color<int16_t> c2(c1);
	REQUIRE(c2 == Color<int16_t>(32767, 0, -32768, 32767));
}


TEST_CASE("Uint16_t -> float", "[Color]") {
	Color<uint16_t> c1(65535, 32767, 0, 65535);
	Color<float> c2(c1);
	REQUIRE(c2.v.Approx() == Color<float>(1, 0.5f, 0, 1).v);
}


TEST_CASE("Int16_t -> float", "[Color]") {
	Color<int16_t> c1(32767, 0, -32768, 32767);
	Color<float> c2(c1);
	REQUIRE(c2.v.Approx() == Color<float>(1, 0.5f, 0, 1).v);
}