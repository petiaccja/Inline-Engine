#include <BaseLibrary/EnumFlag.hpp>

#include <Catch2/catch.hpp>

using namespace inl;


struct FlagData {
	enum EnumT : uint32_t {
		A = 1,
		B = 2,
		C = 4,
	};
};


using Flag = EnumFlag_Helper<FlagData>;

uint32_t ToInteger(const Flag& flag) {
	return uint32_t(Flag::EnumT(flag));
}


TEST_CASE("EnumFlag - Initialize empty", "[EnumFlag]") {
	Flag flag;
	REQUIRE(flag.Empty());
	REQUIRE(ToInteger(flag) == 0);
}


TEST_CASE("EnumFlag - Initialize list", "[EnumFlag]") {
	Flag flag{ Flag::A, Flag::B };
	REQUIRE(ToInteger(flag) == 3);
}


TEST_CASE("EnumFlag - Contains", "[EnumFlag]") {
	Flag flag{ Flag::A, Flag::B };
	REQUIRE(flag.Contains(Flag::A));
	REQUIRE(flag.Contains(Flag::B));
	REQUIRE(!flag.Contains(Flag::C));
}


TEST_CASE("EnumFlag - Add flags", "[EnumFlag]") {
	Flag flag;
	flag += Flag::A;
	REQUIRE(flag.Contains(Flag::A));
	REQUIRE(!flag.Contains(Flag::B));
	REQUIRE(!flag.Contains(Flag::C));
}


TEST_CASE("EnumFlag - Remove flags", "[EnumFlag]") {
	Flag flag{ Flag::A, Flag::B };
	flag -= Flag::B;
	REQUIRE(flag.Contains(Flag::A));
	REQUIRE(!flag.Contains(Flag::B));
	REQUIRE(!flag.Contains(Flag::C));
}


TEST_CASE("EnumFlag - Flag intersection", "[EnumFlag]") {
	Flag flag1{ Flag::A, Flag::B };
	Flag flag2{ Flag::B, Flag::C };
	Flag intersection = flag1 & flag2;
	REQUIRE(!intersection.Contains(Flag::A));
	REQUIRE(intersection.Contains(Flag::B));
	REQUIRE(!intersection.Contains(Flag::C));
}