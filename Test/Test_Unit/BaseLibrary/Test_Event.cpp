#include <BaseLibrary/Event.hpp>

#include <Catch2/catch.hpp>

using namespace inl;


class Incrementer {
public:
	void operator()(int& value) {
		++value;
	}
};

void Increment(int& value) {
	++value;
}


TEST_CASE("Add and fire", "[Event]") {
	int value = 0;
	Incrementer inc;

	Event<int&> evt;
	evt += Increment;
	evt += Delegate<void(int&)>{&Incrementer::operator(), &inc};

	evt(value);

	REQUIRE(value == 2);
}

TEST_CASE("Add and remove", "[Event]") {
	int value = 0;
	Incrementer inc;

	Event<int&> evt;
	evt += Increment;
	evt += Delegate<void(int&)>{&Incrementer::operator(), &inc};
	evt -= Increment;
	evt -= Delegate<void(int&)>{&Incrementer::operator(), &inc};

	evt(value);

	REQUIRE(value == 0);
}

TEST_CASE("Copy and fire", "[Event]") {
	int value = 0;
	Incrementer inc;

	Event<int&> evt;
	evt += Increment;
	evt += Delegate<void(int&)>{&Incrementer::operator(), &inc};

	Event<int&> evt2 = evt;
	evt2(value);

	REQUIRE(value == 2);
}