#include <BaseLibrary/Container/PolymorphicVector.hpp>

#include <Catch2/catch.hpp>
#include <array>
#include <typeindex>

using namespace inl;


class Base {
public:
	virtual std::type_index Type() const { return typeid(Base); }
};

class DerivedA : public Base {
public:
	std::type_index Type() const override { return typeid(DerivedA); }
};

class DerivedB : public Base {
public:
	std::type_index Type() const override { return typeid(DerivedB); }
};


TEST_CASE("Polymorphic vector I can't be arsed", "[BaseLibrary:PolymorphicVector]") {
	PolymorphicVector<Base> v = {
		Base{},
		DerivedA{},
		DerivedB{},
	};

	std::array<std::type_index, 3> expected = {
		typeid(Base),
		typeid(DerivedA),
		typeid(DerivedB),
	};

	auto expectedIt = expected.begin();
	for (auto it = v.begin(); it != v.end(); ++it, ++expectedIt) {
		REQUIRE(it->Type() == *expectedIt);
	}
}