#include <GraphicsEngine/Scene/EntityCollection.hpp>

#include <Catch2/catch.hpp>

using namespace inl::gxeng;


class TestEntity : public Entity {
public:
	int value = 0;
};


TEST_CASE("Add/remove", "[EntityCollection]") {
	EntityCollection<TestEntity> entities;
	TestEntity entity;
	entities.Add(&entity);
	REQUIRE(entity.GetCollection() == &entities);
	REQUIRE(entities.Size() == 1);
	entities.Remove(&entity);
	REQUIRE(entity.GetCollection() == nullptr);
	REQUIRE(entities.Size() == 0);
}



TEST_CASE("Entity dies", "[EntityCollection]") {
	EntityCollection<TestEntity> entities;
	{
		TestEntity entity;
		entities.Add(&entity);
		REQUIRE(entity.GetCollection() == &entities);
		REQUIRE(entities.Size() == 1);
	}
	REQUIRE(entities.Size() == 0);
}


TEST_CASE("Collection dies", "[EntityCollection]") {
	TestEntity entity;
	{
		EntityCollection<TestEntity> entities;
		entities.Add(&entity);
		REQUIRE(entity.GetCollection() == &entities);
		REQUIRE(entities.Size() == 1);
	}
	REQUIRE(entity.GetCollection() == nullptr);
}