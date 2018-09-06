#include <BaseLibrary/UniqueIdGenerator.hpp>

#include <Catch2/catch.hpp>

using namespace inl;


TEST_CASE("Multiple", "[UniqueIdGenerator]") {
	UniqueIdGenerator<int> gen;
	UniqueId id1 = gen(6);
	UniqueId id2 = gen(5);
	UniqueId id3 = gen(6);

	REQUIRE(id1 == id3);
}


TEST_CASE("Different", "[UniqueIdGenerator]") {
	UniqueIdGenerator<int> gen;
	UniqueId id1 = gen(6);
	UniqueId id2 = gen(5);

	REQUIRE(id1 != id2);
}

TEST_CASE("Reset", "[UniqueIdGenerator]") {
	UniqueIdGenerator<int> gen;
	UniqueId id1 = gen(6);
	UniqueId id2 = gen(5);
	REQUIRE(gen.DataBaseSize() == 2);
	gen.Reset();

	REQUIRE(gen.DataBaseSize() == 0);
}