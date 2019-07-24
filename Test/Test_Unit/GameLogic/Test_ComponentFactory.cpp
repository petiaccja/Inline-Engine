#include "Components.hpp"

#include <GameLogic/ComponentFactory.hpp>

#include <Catch2/catch.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/binary.hpp>
#include <sstream>

using namespace inl::game;


TEST_CASE("ComponentFactory - Registration", "[GameLogic]") {
	REQUIRE(ComponentFactory_Singleton::GetInstance().IsRegistered<FooComponent>());
	REQUIRE(ComponentFactory_Singleton::GetInstance().IsRegistered<BarComponent>());
	REQUIRE(ComponentFactory_Singleton::GetInstance().IsRegistered<BazComponent>());
}


TEST_CASE("ComponentFactory - Create", "[GameLogic]") {
	World world;
	Entity& entity = *world.CreateEntity();
	ComponentFactory_Singleton::GetInstance().Create(entity, "FooComponent");
	REQUIRE(entity.HasComponent<FooComponent>());
}


TEST_CASE("ComponentFactory - Create with special factory", "[GameLogic]") {
	World world;
	Entity& entity = *world.CreateEntity();
	auto& factory = ComponentFactory_Singleton::GetInstance();
	auto& specialFactory = factory.GetClassFactory<SpecialComponent, SpecialFactory>();
	specialFactory.Configure(16.0f);
	factory.Create(entity, "SpecialComponent");

	REQUIRE(entity.HasComponent<SpecialComponent>());
	REQUIRE(entity.GetFirstComponent<SpecialComponent>().value == 16.0f);
}


TEST_CASE("ComponentFactory - Variant serializer", "[GameLogic]") {
	using ArchiveMix = VariantOutputArchive<cereal::JSONOutputArchive, cereal::BinaryOutputArchive>;
	std::stringstream ss1, ss2;
	FooComponent serializable{ 5.0f };
	{
		ArchiveMix archiveMix{ std::in_place_type<cereal::JSONOutputArchive>, ss1 };
		cereal::JSONOutputArchive jsonArchive{ ss2 };
		archiveMix(serializable);
		jsonArchive(serializable);	
		archiveMix(12.f);
		jsonArchive(12.f);
	}
	std::string s1 = ss1.str();
	std::string s2 = ss2.str();
	REQUIRE(s1 == s2);
}