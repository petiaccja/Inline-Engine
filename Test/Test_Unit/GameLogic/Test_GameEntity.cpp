#include <Catch2/catch.hpp>
#include <GameLogic/Component.hpp>
#include <GameLogic/GameEntity.hpp>

using namespace inl::game;


class DummyComponent : public Component {
	float value;
};


TEST_CASE("GameEntity - AddComponent", "[GameLogic]") {
	DummyComponent component;
	GameEntity entity;
	entity.AddComponent(component);
	REQUIRE(component.GetEntity() == &entity);
}


TEST_CASE("GameEntity - RemoveComponent", "[GameLogic]") {
	DummyComponent component;
	GameEntity entity;
	entity.AddComponent(component);
	entity.RemoveComponent(component);
	REQUIRE(component.GetEntity() == nullptr);
}


TEST_CASE("GameEntity - Destruct", "[GameLogic]") {
	DummyComponent component;
	{
		GameEntity entity;
		entity.AddComponent(component);
	}
	REQUIRE(component.GetEntity() == nullptr);
}


TEST_CASE("GameEntity - Get component success", "[GameLogic]") {
	DummyComponent component;
	GameEntity entity;
	entity.AddComponent(component);
	REQUIRE_NOTHROW(entity.GetComponents<DummyComponent>());
	auto range = entity.GetComponents<DummyComponent>();
	REQUIRE(std::distance(range.first, range.second) == 1);
	REQUIRE(&*range.first == &component);
}


TEST_CASE("GameEntity - Get component fail", "[GameLogic]") {
	GameEntity entity;
	auto range = entity.GetComponents<DummyComponent>();
	REQUIRE(std::distance(range.first, range.second) == 0);
}


TEST_CASE("GameEntity - Get first component success", "[GameLogic]") {
	DummyComponent component;
	GameEntity entity;
	entity.AddComponent(component);
	REQUIRE_NOTHROW(entity.GetFirstComponent<DummyComponent>());
	REQUIRE(&entity.GetFirstComponent<DummyComponent>() == &component);
}


TEST_CASE("GameEntity - Get first component fail", "[GameLogic]") {
	GameEntity entity;
	REQUIRE_THROWS(entity.GetFirstComponent<DummyComponent>());
}