#include "Components.hpp"

#include <GameLogic/Level.hpp>

#include <Catch2/catch.hpp>
#include <sstream>

using namespace inl::game;


TEST_CASE("Save", "[GameLogic:Level]") {
	Scene scene;
	scene.CreateEntity(FooComponent{ 16.f }, BarComponent{ 32.f });
	scene.CreateEntity(BazComponent{ 64.f });

	Level level{ scene };

	std::stringstream output;

	{
		LevelOutputArchive archive{ std::in_place_type<cereal::JSONOutputArchive>, output };
		level.Save(archive, ComponentFactory_Singleton::GetInstance());
	}

	std::string raw = output.str();
	REQUIRE(raw.find("16") != raw.npos);
	REQUIRE(raw.find("32") != raw.npos);
	REQUIRE(raw.find("64") != raw.npos);
}


TEST_CASE("Save-Load cycle", "[GameLogic:Level]") {
	Scene savedScene;
	savedScene.CreateEntity(FooComponent{ 16.f }, BarComponent{ 32.f });
	savedScene.CreateEntity(BazComponent{ 64.f });

	Level savedLevel{ savedScene };

	std::stringstream output;
	{
		LevelOutputArchive archive{ std::in_place_type<cereal::JSONOutputArchive>, output };
		savedLevel.Save(archive, ComponentFactory_Singleton::GetInstance());
	}

	Scene loadedScene;
	Level loadedLevel{ loadedScene };

	output.seekg(0, std::ios::beg);
	output.clear();

	{
		LevelInputArchive inputArchive{ std::in_place_type<cereal::JSONInputArchive>, output };
		loadedLevel.Load(inputArchive, ComponentFactory_Singleton::GetInstance());
	}

	int numEntities = 0;
	int numFooComponents = 0;
	int numBarComponents = 0;
	int numBazComponents = 0;
	for (auto& entity : loadedScene) {
		++numEntities;
		numFooComponents += (int)entity.HasComponent<FooComponent>();
		numBarComponents += (int)entity.HasComponent<BarComponent>();
		numBazComponents += (int)entity.HasComponent<BazComponent>();
	}
	REQUIRE(numEntities == 2);
	REQUIRE(numFooComponents == 1);
	REQUIRE(numBarComponents == 1);
	REQUIRE(numBazComponents == 1);
}
