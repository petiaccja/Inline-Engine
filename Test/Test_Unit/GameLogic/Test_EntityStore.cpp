#include <Catch2/catch.hpp>
#include <GameLogic/Component.hpp>
#include "GameLogic/EntityStore.hpp"

using namespace inl::game;


class FooComponent : public Component {
public:
	float value = 0.0f;
};

class BarComponent : public Component {
public:
	float value = 1.0f;
};

class BazComponent : public Component {
public:
	float value = 2.0f;
};



TEST_CASE("EntityStore - Extemd", "[GameLogic]") {
	EntityStore store;
	store.Extend<FooComponent>();
	store.Extend<BarComponent>();
	store.Extend<BazComponent>();

	REQUIRE(false);
}
