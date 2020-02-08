#include "Components.hpp"
#include "Systems.hpp"

#include <GameLogic/Scene.hpp>
#include <GameLogic/Simulation.hpp>

#include <Catch2/catch.hpp>

using namespace inl::game;


TEST_CASE("Insert/PushBack systems", "[GameLogic:Simulation]") {
	Simulation sm;
	sm.PushBack(DoubleFooToBarSystem{});
	sm.PushBack(StandaloneSystem{});
	sm.Insert(sm.begin() + 1, StandaloneSystem{});

	REQUIRE(sm.Size() == 3);
	REQUIRE(typeid(sm[0]) == typeid(DoubleFooToBarSystem));
	REQUIRE(typeid(sm[1]) == typeid(StandaloneSystem));
	REQUIRE(typeid(sm[2]) == typeid(StandaloneSystem));
}


TEST_CASE("Remove system by type", "[GameLogic:Simulation]") {
	Simulation sm;
	sm.PushBack(DoubleFooToBarSystem{});
	sm.PushBack(StandaloneSystem{});
	sm.Insert(sm.begin() + 1, StandaloneSystem{});

	sm.Remove<DoubleFooToBarSystem>();
	REQUIRE(sm.Size() == 2);
	REQUIRE(typeid(sm[0]) == typeid(StandaloneSystem));
	REQUIRE(typeid(sm[1]) == typeid(StandaloneSystem));
}


TEST_CASE("Remove ALL systems by type", "[GameLogic:Simulation]") {
	Simulation sm;
	sm.PushBack(DoubleFooToBarSystem{});
	sm.PushBack(StandaloneSystem{});
	sm.Insert(sm.begin() + 1, StandaloneSystem{});

	sm.RemoveAll<StandaloneSystem>();
	REQUIRE(sm.Size() == 1);
	REQUIRE(typeid(sm[0]) == typeid(DoubleFooToBarSystem));
}


TEST_CASE("Remove concrete system", "[GameLogic:Simulation]") {
	Simulation sm;
	sm.PushBack(DoubleFooToBarSystem{});
	sm.PushBack(StandaloneSystem{});
	sm.Insert(sm.begin() + 1, StandaloneSystem{});

	sm.Remove(sm[1]);
	REQUIRE(sm.Size() == 2);
	REQUIRE(typeid(sm[0]) == typeid(DoubleFooToBarSystem));
	REQUIRE(typeid(sm[1]) == typeid(StandaloneSystem));
}


TEST_CASE("Splice system to another point", "[GameLogic:Simulation]") {
	Simulation sm;
	sm.PushBack(DoubleFooToBarSystem{});
	sm.PushBack(StandaloneSystem{});
	sm.Insert(sm.begin() + 1, StandaloneSystem{});

	sm.Splice(sm.begin(), --sm.end());
	REQUIRE(sm.Size() == 3);
	REQUIRE(typeid(sm[0]) == typeid(StandaloneSystem));
	REQUIRE(typeid(sm[1]) == typeid(DoubleFooToBarSystem));
	REQUIRE(typeid(sm[2]) == typeid(StandaloneSystem));
}


TEST_CASE("Iterate systems", "[GameLogic:Simulation]") {
	Simulation sm;
	sm.PushBack(DoubleFooToBarSystem{});
	sm.PushBack(StandaloneSystem{});
	sm.Insert(sm.begin() + 1, StandaloneSystem{});

	int count = 0;
	for (auto& system : sm) {
		++count;
	}
	REQUIRE(count == 3);
}


TEST_CASE("Iterate systems backwards", "[GameLogic:Simulation]") {
	Simulation sm;
	sm.PushBack(DoubleFooToBarSystem{});
	sm.PushBack(StandaloneSystem{});
	sm.Insert(sm.begin() + 1, StandaloneSystem{});

	int count = 0;
	auto first = sm.begin();
	auto last = sm.end();
	do {
		--last;
		auto& system = *last;
		REQUIRE(&system != nullptr);
		++count;
	} while (first != last);
	REQUIRE(count == 3);
}


TEST_CASE("Get ALL systems by type", "[GameLogic:Simulation]") {
	Simulation sm;
	sm.PushBack(DoubleFooToBarSystem{});
	sm.PushBack(StandaloneSystem{});
	sm.Insert(sm.begin() + 1, StandaloneSystem{});

	int count = 0;
	for (StandaloneSystem& system : sm.GetAll<StandaloneSystem>()) {
		REQUIRE(typeid(system) == typeid(StandaloneSystem));
		++count;
	}
	REQUIRE(count == 2);
}


TEST_CASE("Get first system by type", "[GameLogic:Simulation]") {
	Simulation sm;
	sm.PushBack(DoubleFooToBarSystem{});
	sm.PushBack(StandaloneSystem{});
	sm.Insert(sm.begin() + 1, StandaloneSystem{});

	auto& system = sm.Get<StandaloneSystem>();
	REQUIRE(typeid(system) == typeid(StandaloneSystem));
}


TEST_CASE("Get first system by type FAIL", "[GameLogic:Simulation]") {
	Simulation sm;
	sm.PushBack(StandaloneSystem{});
	sm.Insert(sm.begin() + 1, StandaloneSystem{});

	REQUIRE_THROWS(sm.Get<DoubleFooToBarSystem>());
}


TEST_CASE("Run systems on Scene", "[GameLogic:Simulation]") {
	Scene scene;
	Simulation sm;

	Entity& entity1 = scene.CreateEntity(FooComponent{ 12.f }, BarComponent{ 0.0f });
	Entity& entity2 = scene.CreateEntity(FooComponent{ 12.f }, BarComponent{ 0.0f }, BazComponent{ 0.0f });

	sm.PushBack(DoubleFooToBarSystem{});
	sm.PushBack(StandaloneSystem{});

	sm.Run(scene, 0.0f);

	REQUIRE(entity1.GetFirstComponent<BarComponent>().value == 24.f);
	REQUIRE(entity2.GetFirstComponent<BarComponent>().value == 24.f);
	REQUIRE(sm.Get<StandaloneSystem>().content == "use renewables;");
}