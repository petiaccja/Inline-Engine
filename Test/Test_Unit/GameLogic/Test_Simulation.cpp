#include "Components.hpp"
#include "Systems.hpp"

#include <GameLogic/Scene.hpp>
#include <GameLogic/Simulation.hpp>

#include <Catch2/catch.hpp>

using namespace inl::game;



TEST_CASE("Run systems", "[GameLogic:Simulation]") {
	Scene scene;
	Simulation sm;

	Entity& entity1 = scene.CreateEntity(FooComponent{ 12.f }, BarComponent{ 0.0f });
	Entity& entity2 = scene.CreateEntity(FooComponent{ 12.f }, BarComponent{ 0.0f }, BazComponent{ 0.0f });

	sm.systems = {
		DoubleFooToBarSystem{},
		StandaloneSystem{},
	};

	sm.Run(scene, 0.0f);

	REQUIRE(entity1.GetFirstComponent<BarComponent>().value == 24.f);
	REQUIRE(entity2.GetFirstComponent<BarComponent>().value == 24.f);
	REQUIRE(dynamic_cast<StandaloneSystem&>(sm.systems[1]).content == "use renewables;");
}


TEST_CASE("Run hooks", "[GameLogic:Simulation]") {

	Scene scene;
	Simulation sm;

	sm.systems = {
		MessageSystem{},
		StandaloneSystem{},
	};
	sm.hooks = {
		MessageHook{},
	};	

	sm.Run(scene, 0.0f);
	
	REQUIRE(dynamic_cast<StandaloneSystem&>(sm.systems[1]).content == "use renewables;");
	REQUIRE(dynamic_cast<MessageHook&>(sm.hooks[0]).message == "MAKE ME UPPERCASE");	
}