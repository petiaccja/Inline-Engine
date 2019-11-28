#include <BaseLibrary/DynamicTuple.hpp>

#include <Catch2/catch.hpp>

using namespace inl;


TEST_CASE("Access empty", "[DynamicTuple]") {
	DynamicTuple tuple;
	REQUIRE_THROWS(tuple.Get<float>());
}


TEST_CASE("Add", "[DynamicTuple]") {
	DynamicTuple tuple;
	tuple.Insert(float(42.f));
	REQUIRE(tuple.Has<float>());
}


TEST_CASE("Add repeated", "[DynamicTuple]") {
	DynamicTuple tuple;
	tuple.Insert(float(42.f));
	REQUIRE_THROWS(tuple.Insert(float(42.f)));
}


TEST_CASE("Add multiple", "[DynamicTuple]") {
	DynamicTuple tuple;
	tuple.Insert(float(42.f));
	tuple.Insert(int(7));

	REQUIRE(tuple.Has<float>());
	REQUIRE(tuple.Has<int>());
}


TEST_CASE("Get & change", "[DynamicTuple]") {
	DynamicTuple tuple;
	tuple.Insert(float(42.f));
	tuple.Get<float>() *= 2.0f;
	REQUIRE(tuple.Get<float>() == 84.f);
}


TEST_CASE("Remove", "[DynamicTuple]") {
	DynamicTuple tuple;
	tuple.Insert(float(42.f));
	tuple.Insert(int(7));

	tuple.Erase<float>();

	REQUIRE(!tuple.Has<float>());
	REQUIRE(tuple.Has<int>());
}


TEST_CASE("Clear", "[DynamicTuple]") {
	DynamicTuple tuple;
	tuple.Insert(float(42.f));
	tuple.Insert(int(7));

	tuple.Clear();

	REQUIRE(!tuple.Has<float>());
	REQUIRE(!tuple.Has<int>());
}