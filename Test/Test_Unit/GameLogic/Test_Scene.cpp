#include "Components.hpp"

#include <GameLogic/Scene.hpp>

#include <Catch2/catch.hpp>

using namespace inl::game;


TEST_CASE("AddEntity", "[GameLogic:Scene]") {
	Scene scene;
	auto entity = scene.CreateEntity(FooComponent{}, BarComponent{}, BazComponent{});

	REQUIRE(entity->GetSet()->matrix.entities.size() == 1);
	REQUIRE(entity->GetIndex() == 0);
	REQUIRE(entity->GetScene() == &scene);
}


TEST_CASE("Delete entity", "[GameLogic:Scene]") {
	Scene scene;
	auto entity1 = scene.CreateEntity(FooComponent{}, BarComponent{}, BazComponent{});
	auto entity2 = scene.CreateEntity(FooComponent{}, BarComponent{}, BazComponent{});

	scene.DeleteEntity(*entity1);

	REQUIRE(entity2->GetSet()->matrix.entities.size() == 1);
	REQUIRE(entity2->GetIndex() == 0);
	REQUIRE(entity2->GetScene() == &scene);
}


TEST_CASE("Iterate entities", "[GameLogic:Scene]") {
	Scene scene;
	scene.CreateEntity(FooComponent{}, BarComponent{}, BazComponent{});
	scene.CreateEntity(FooComponent{}, BarComponent{}, BazComponent{});
	scene.CreateEntity(FooComponent{}, BazComponent{});
	auto* deleted = scene.CreateEntity(FooComponent{}, BarComponent{});

	scene.DeleteEntity(*deleted);

	int count = 0;
	for (auto& entity : scene) {
		REQUIRE(entity.GetScene() == &scene);
		++count;
	}
	REQUIRE(count == 3);
}


TEST_CASE("Iterate backwards", "[GameLogic:Scene]") {
	Scene scene;
	scene.CreateEntity(FooComponent{}, BarComponent{}, BazComponent{});
	scene.CreateEntity(FooComponent{}, BarComponent{}, BazComponent{});
	scene.CreateEntity(FooComponent{}, BazComponent{});
	auto* deleted = scene.CreateEntity(FooComponent{}, BarComponent{});

	scene.DeleteEntity(*deleted);

	int count = 0;
	auto first = scene.begin();
	auto last = scene.end();
	do {
		--last;
		REQUIRE((*last).GetScene() == &scene);
		++count;
	} while (last != first);

	REQUIRE(count == 3);
}


TEST_CASE("Get stores", "[GameLogic:Scene]") {
	Scene scene;
	scene.CreateEntity(FooComponent{}, BarComponent{}, BazComponent{});
	scene.CreateEntity(FooComponent{}, BarComponent{}, BazComponent{});
	scene.CreateEntity(FooComponent{}, BazComponent{});

	int count = 0;
	for (auto& matrix : scene.GetMatrices({ typeid(FooComponent), typeid(BarComponent) })) {
		++count;
	}
	REQUIRE(count == 1);

	count = 0;
	for (auto& matrix : scene.GetMatrices({ typeid(FooComponent) })) {
		++count;
	}
	REQUIRE(count == 2);

	count = 0;
	for (auto& matrix : scene.GetMatrices({ typeid(float) })) {
		++count;
	}
	REQUIRE(count == 0);
}


TEST_CASE("Add multiple entities", "[GameLogic:Scene]") {
	Scene scene;
	auto entity0 = scene.CreateEntity(FooComponent{}, BarComponent{}, BazComponent{});
	auto entity1 = scene.CreateEntity(FooComponent{}, BarComponent{}, BazComponent{});

	REQUIRE(entity0->GetSet()->matrix.entities.size() == 2);
	REQUIRE(entity0->GetIndex() == 0);
	REQUIRE(entity0->GetScene() == &scene);

	REQUIRE(entity1->GetSet()->matrix.entities.size() == 2);
	REQUIRE(entity1->GetIndex() == 1);
	REQUIRE(entity1->GetScene() == &scene);
}


TEST_CASE("Add different entities", "[GameLogic:Scene]") {
	Scene scene;
	auto entity0 = scene.CreateEntity(FooComponent{}, BarComponent{}, BazComponent{});
	auto entity1 = scene.CreateEntity(FooComponent{}, BarComponent{});

	REQUIRE(entity0->GetSet()->matrix.entities.size() == 1);
	REQUIRE(entity0->GetIndex() == 0);
	REQUIRE(entity0->GetScene() == &scene);

	REQUIRE(entity1->GetSet()->matrix.entities.size() == 1);
	REQUIRE(entity1->GetIndex() == 0);
	REQUIRE(entity1->GetScene() == &scene);
}


TEST_CASE("Add component new", "[GameLogic:Scene]") {
	Scene scene;
	auto entity0 = scene.CreateEntity(FooComponent{}, BarComponent{}, BazComponent{});
	auto entity1 = scene.CreateEntity(FooComponent{}, BarComponent{});
	scene.AddComponent(*entity0, BazComponent{});

	REQUIRE(entity0->GetSet()->matrix.entities.size() == 1);
	REQUIRE(entity0->GetSet()->matrix.types.size() == 4);
	REQUIRE(entity0->GetIndex() == 0);
	REQUIRE(entity0->GetScene() == &scene);

	REQUIRE(entity1->GetSet()->matrix.entities.size() == 1);
	REQUIRE(entity1->GetSet()->matrix.types.size() == 2);
	REQUIRE(entity1->GetIndex() == 0);
	REQUIRE(entity1->GetScene() == &scene);
}


TEST_CASE("Add component merge", "[GameLogic:Scene]") {
	Scene scene;
	auto entity0 = scene.CreateEntity(FooComponent{}, BarComponent{}, BazComponent{});
	auto entity1 = scene.CreateEntity(FooComponent{}, BarComponent{});
	scene.AddComponent(*entity1, BazComponent{});

	REQUIRE(entity0->GetSet()->matrix.entities.size() == 2);
	REQUIRE(entity0->GetIndex() == 0);
	REQUIRE(entity0->GetScene() == &scene);

	REQUIRE(entity1->GetSet() == entity0->GetSet());
	REQUIRE(entity1->GetIndex() == 1);
	REQUIRE(entity1->GetScene() == &scene);
}


TEST_CASE("Remove component new", "[GameLogic:Scene]") {
	Scene scene;
	auto entity0 = scene.CreateEntity(FooComponent{}, BarComponent{}, BazComponent{});
	auto entity1 = scene.CreateEntity(FooComponent{}, BarComponent{});
	scene.RemoveComponent<BarComponent>(*entity1);

	REQUIRE(entity0->GetSet()->matrix.entities.size() == 1);
	REQUIRE(entity0->GetSet()->matrix.types.size() == 3);
	REQUIRE(entity0->GetIndex() == 0);
	REQUIRE(entity0->GetScene() == &scene);

	REQUIRE(entity1->GetSet()->matrix.entities.size() == 1);
	REQUIRE(entity1->GetSet()->matrix.types.size() == 1);
	REQUIRE(entity1->GetIndex() == 0);
	REQUIRE(entity1->GetScene() == &scene);
}


TEST_CASE("Remove component merge", "[GameLogic:Scene]") {
	Scene scene;
	auto entity0 = scene.CreateEntity(FooComponent{}, BarComponent{}, BazComponent{});
	auto entity1 = scene.CreateEntity(FooComponent{}, BarComponent{});
	scene.RemoveComponent<BazComponent>(*entity0);

	REQUIRE(entity0->GetSet()->matrix.entities.size() == 2);
	REQUIRE(entity0->GetSet()->matrix.types.size() == 2);
	REQUIRE(entity0->GetIndex() == 1);
	REQUIRE(entity0->GetScene() == &scene);

	REQUIRE(entity1->GetSet() == entity0->GetSet());
	REQUIRE(entity1->GetIndex() == 0);
	REQUIRE(entity1->GetScene() == &scene);
}


TEST_CASE("Create empty entities", "[GameLogic:Scene]") {
	Scene scene;
	auto entity0 = scene.CreateEntity();
	scene.AddComponent(*entity0, BazComponent{});
	auto entity1 = scene.CreateEntity();
	scene.AddComponent(*entity1, BarComponent{});

	REQUIRE(entity0->HasComponent<BazComponent>());
	REQUIRE(entity1->HasComponent<BarComponent>());
}


TEST_CASE("Concatenating worlds", "[GameLogic:Scene]") {
	Scene world1;
	Scene world2;

	Entity* entity11 = world1.CreateEntity(FooComponent{ 11 }, BarComponent{ 11 });
	Entity* entity12 = world1.CreateEntity(FooComponent{ 12 }, BazComponent{ 12 });

	Entity* entity21 = world2.CreateEntity(FooComponent{ 21 }, BarComponent{ 21 });
	Entity* entity22 = world2.CreateEntity(BarComponent{ 22 }, BazComponent{ 22 });

	world1 += std::move(world2);

	REQUIRE(entity11->GetScene() == &world1);
	REQUIRE(entity12->GetScene() == &world1);
	REQUIRE(entity21->GetScene() == &world1);
	REQUIRE(entity22->GetScene() == &world1);

	REQUIRE(entity21->GetIndex() == 1);
	REQUIRE(entity22->GetIndex() == 0);
}
