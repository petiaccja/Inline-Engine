#include <Catch2/catch.hpp>
#include <GameLogic/Component.hpp>
#include <GameLogic/GameEntity.hpp>

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


TEST_CASE("GameEntity - AddComponent", "[GameLogic]") {
	FooComponent component;
	GameEntity entity;
	entity.AddComponent(component);
	REQUIRE(component.GetEntity() == &entity);
}


TEST_CASE("GameEntity - RemoveComponent", "[GameLogic]") {
	FooComponent component;
	GameEntity entity;
	entity.AddComponent(component);
	entity.RemoveComponent(component);
	REQUIRE(component.GetEntity() == nullptr);
}


TEST_CASE("GameEntity - Destruct", "[GameLogic]") {
	FooComponent component;
	{
		GameEntity entity;
		entity.AddComponent(component);
	}
	REQUIRE(component.GetEntity() == nullptr);
}


TEST_CASE("GameEntity - Get component success", "[GameLogic]") {
	FooComponent component;
	GameEntity entity;
	entity.AddComponent(component);
	REQUIRE_NOTHROW(entity.GetComponents<FooComponent>());
	auto range = entity.GetComponents<FooComponent>();
	REQUIRE(std::distance(range.first, range.second) == 1);
	REQUIRE(&*range.first == &component);
}


TEST_CASE("GameEntity - Get component fail", "[GameLogic]") {
	GameEntity entity;
	auto range = entity.GetComponents<FooComponent>();
	REQUIRE(std::distance(range.first, range.second) == 0);
}


TEST_CASE("GameEntity - Get first component success", "[GameLogic]") {
	FooComponent component;
	GameEntity entity;
	entity.AddComponent(component);
	REQUIRE_NOTHROW(entity.GetFirstComponent<FooComponent>());
	REQUIRE(&entity.GetFirstComponent<FooComponent>() == &component);
}


TEST_CASE("GameEntity - Get first component fail", "[GameLogic]") {
	GameEntity entity;
	REQUIRE_THROWS(entity.GetFirstComponent<FooComponent>());
}


TEST_CASE("GameEntity - Get component range", "[GameLogic]") {
	FooComponent foo1, foo2;
	BarComponent bar1, bar2, bar3;
	BazComponent baz1, baz2, baz3;

	GameEntity entity;
	entity.AddComponent(foo1);
	entity.AddComponent(foo2);
	entity.AddComponent(bar1);
	entity.AddComponent(bar2);
	entity.AddComponent(bar3);
	entity.AddComponent(baz1);
	entity.AddComponent(baz2);
	entity.AddComponent(baz3);

	auto fooRange = entity.GetComponents<FooComponent>();
	auto barRange = entity.GetComponents<BarComponent>();
	auto bazRange = entity.GetComponents<BazComponent>();
	REQUIRE(std::distance(fooRange.first, fooRange.second) == 2);
	REQUIRE(std::distance(barRange.first, barRange.second) == 3);
	REQUIRE(std::distance(bazRange.first, bazRange.second) == 3);

	REQUIRE(fooRange.first->value == 0.0f);
	REQUIRE(barRange.first->value == 1.0f);
	REQUIRE(bazRange.first->value == 2.0f);
}