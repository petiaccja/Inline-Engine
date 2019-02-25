#include <BaseLibrary/BitOperations.hpp>

#include <Catch2/catch.hpp>

using namespace inl;


class Base {};
class Derived : Base {};
class DerivedVirtual : virtual Base {};



TEST_CASE("C-Trailing-Z 32", "[BitOperations]") {
	uint32_t number = 0b00000000'00000000'00000001'00000000;
	REQUIRE(CountTrailingZeros(number) == 8);
}



TEST_CASE("C-Leading-Z 32", "[BitOperations]") {
	uint32_t number = 0b00000000'00000000'00000001'00000000;
	REQUIRE(CountLeadingZeros(number) == 23);
}



TEST_CASE("C-Trailing-Z 32 all zeros", "[BitOperations]") {
	uint32_t number = 0;
	REQUIRE(CountTrailingZeros(number) == -1);
}



TEST_CASE("C-Leading-Z 32 all zeros", "[BitOperations]") {
	uint32_t number = 0;
	REQUIRE(CountTrailingZeros(number) == -1);
}


TEST_CASE("C-Trailing-Z 64", "[BitOperations]") {
	uint64_t number = 0b00000000'00000000'00000001'00000000'00000000'00000000'00000001'00000000;
	REQUIRE(CountTrailingZeros(number) == 8);
}



TEST_CASE("C-Leading-Z 64", "[BitOperations]") {
	uint64_t number = 0b00000000'00000000'00000001'00000000'00000000'00000000'00000001'00000000;
	REQUIRE(CountLeadingZeros(number) == 23);
}



TEST_CASE("C-Trailing-Z 64 all zeros", "[BitOperations]") {
	uint64_t number = 0;
	REQUIRE(CountTrailingZeros(number) == -1);
}



TEST_CASE("C-Leading-Z 64 all zeros", "[BitOperations]") {
	uint64_t number = 0;
	REQUIRE(CountTrailingZeros(number) == -1);
}


TEST_CASE("BTS 32 false", "[BitOperations]") {
	uint32_t number = 0;
	REQUIRE(BitTestAndSet(number, 8) == false);
	REQUIRE(number == 256);
}


TEST_CASE("BTS 32 true", "[BitOperations]") {
	uint32_t number = ~uint32_t(0);
	REQUIRE(BitTestAndSet(number, 8) == true);
	REQUIRE(number == ~uint32_t(0));
}


TEST_CASE("BTS 64 false", "[BitOperations]") {
	uint64_t number = 0;
	REQUIRE(BitTestAndSet(number, 8) == false);
	REQUIRE(number == 256);
}


TEST_CASE("BTS 64 true", "[BitOperations]") {
	uint64_t number = ~uint64_t(0);
	REQUIRE(BitTestAndSet(number, 8) == true);
	REQUIRE(number == ~uint64_t(0));
}


TEST_CASE("BTC 32 false", "[BitOperations]") {
	uint32_t number = 0;
	REQUIRE(BitTestAndClear(number, 8) == false);
	REQUIRE(number == 0);
}


TEST_CASE("BTC 32 true", "[BitOperations]") {
	uint32_t number = ~uint32_t(0);
	REQUIRE(BitTestAndClear(number, 8) == true);
	REQUIRE(number == 0xFFFFFEFFu);
}


TEST_CASE("BTC 64 false", "[BitOperations]") {
	uint64_t number = 0;
	REQUIRE(BitTestAndClear(number, 8) == false);
	REQUIRE(number == 0);
}


TEST_CASE("BTC 64 true", "[BitOperations]") {
	uint64_t number = ~uint64_t(0);
	REQUIRE(BitTestAndClear(number, 8) == true);
	REQUIRE(number == 0xFFFFFFFF'FFFFFEFFu);
}