#include "Components.hpp"

#include <GameLogic/EntitySchemeSet.hpp>

#include <Catch2/catch.hpp>
#include <array>

using namespace inl::game;


TEST_CASE("Construct", "[GameLogic:EntitySchemeSet]") {
	Scene scene;
	EntitySchemeSet set(scene);

	REQUIRE(&set.GetParent() == &scene);
	REQUIRE(set.Size() == 0);
	REQUIRE(set.GetMatrix().types.size() == 0);
	REQUIRE(set.Empty());
}


TEST_CASE("Set types", "[GameLogic:EntitySchemeSet]") {
	Scene scene;
	EntitySchemeSet set(scene);

	set.SetComponentTypes<float, int, bool>();

	REQUIRE(set.Size() == 0);
	REQUIRE(set.GetScheme() == ComponentScheme{ typeid(float), typeid(int), typeid(bool) });
	REQUIRE(set.GetMatrix().entities.size() == 0);
	REQUIRE(set.GetMatrix().types.size() == 3);
}


TEST_CASE("Add more types", "[GameLogic:EntitySchemeSet]") {
	Scene scene;
	EntitySchemeSet set(scene);

	set.SetComponentTypes<float, int, bool>();
	set.AddComponentType<double>();

	REQUIRE(set.Size() == 0);
	REQUIRE(set.GetScheme() == ComponentScheme{ typeid(float), typeid(int), typeid(bool), typeid(double) });
	REQUIRE(set.GetMatrix().entities.size() == 0);
	REQUIRE(set.GetMatrix().types.size() == 4);
}


TEST_CASE("Remove types", "[GameLogic:EntitySchemeSet]") {
	Scene scene;
	EntitySchemeSet set(scene);

	set.SetComponentTypes<float, int, bool>();
	auto indexOfInt = set.GetScheme().Index(typeid(int)).first;
	set.RemoveComponentType(indexOfInt);

	REQUIRE(set.Size() == 0);
	REQUIRE(set.GetScheme() == ComponentScheme{ typeid(float), typeid(bool) });
	REQUIRE(set.GetMatrix().entities.size() == 0);
	REQUIRE(set.GetMatrix().types.size() == 2);
}


TEST_CASE("Copy types", "[GameLogic:EntitySchemeSet]") {
	Scene scene;
	EntitySchemeSet set1(scene);
	EntitySchemeSet set2(scene);

	set1.SetComponentTypes<float, int, bool>();
	set1.Create();
	set1.Create();
	set1.Create();

	set2.Create();

	set2.CopyComponentTypes(set1);
	REQUIRE(set2.Size() == 1);
	REQUIRE(set2.GetScheme() == ComponentScheme{ typeid(float), typeid(int), typeid(bool) });
	REQUIRE(set2.GetMatrix().types.size() == 3);
}


TEST_CASE("Create entity empty", "[GameLogic:EntitySchemeSet]") {
	Scene scene;
	EntitySchemeSet set(scene);

	Entity& ent1 = set.Create();
	Entity& ent2 = set.Create();

	REQUIRE(ent1.GetSet() == &set);
	REQUIRE(ent1.GetScene() == &scene);

	REQUIRE(set.Size() == 2);
	REQUIRE(set.GetMatrix().entities.size() == 0);
	REQUIRE(set.GetMatrix().types.size() == 0);
}


TEST_CASE("Destroy entity empty", "[GameLogic:EntitySchemeSet]") {
	Scene scene;
	EntitySchemeSet set(scene);

	Entity& ent1 = set.Create();
	Entity& ent2 = set.Create();

	set.Destroy(ent1);

	REQUIRE(ent2.GetSet() == &set);
	REQUIRE(ent2.GetScene() == &scene);

	REQUIRE(set.Size() == 1);
	REQUIRE(set.GetMatrix().entities.size() == 0);
	REQUIRE(set.GetMatrix().types.size() == 0);
}


TEST_CASE("Create entity normal", "[GameLogic:EntitySchemeSet]") {
	Scene scene;
	EntitySchemeSet set(scene);
	set.SetComponentTypes<float, bool>();

	Entity& ent1 = set.Create(3.14f, false);
	Entity& ent2 = set.Create(2.72f);

	REQUIRE(ent1.GetFirstComponent<float>() == 3.14f);
	REQUIRE(ent1.GetFirstComponent<bool>() == false);
	REQUIRE(ent2.GetFirstComponent<float>() == 2.72f);
	REQUIRE((ent2.GetFirstComponent<bool>() || true)); // Not initialized, we don't know.

	REQUIRE(set.Size() == 2);
	REQUIRE(set.GetMatrix().entities.size() == 2);
	REQUIRE(set.GetMatrix().types.size() == 2);
}


TEST_CASE("Destroy entity normal", "[GameLogic:EntitySchemeSet]") {
	Scene scene;
	EntitySchemeSet set(scene);
	set.SetComponentTypes<float, bool>();

	Entity& ent1 = set.Create(3.14f, false);
	Entity& ent2 = set.Create(2.72f);

	set.Destroy(ent1);

	REQUIRE(ent2.GetFirstComponent<float>() == 2.72f);
	REQUIRE((ent2.GetFirstComponent<bool>() || true)); // Not initialized, we don't know.

	REQUIRE(set.Size() == 1);
	REQUIRE(set.GetMatrix().entities.size() == 1);
	REQUIRE(set.GetMatrix().types.size() == 2);
}


TEST_CASE("Iteration & indexing", "[GameLogic:EntitySchemeSet]") {
	Scene scene;
	EntitySchemeSet set(scene);

	std::array<Entity*, 3> entities = { nullptr, nullptr, nullptr };
	entities[0] = &set.Create();
	entities[1] = &set.Create();
	entities[2] = &set.Create();

	size_t index = 0;
	for (auto& entity : set) {
		REQUIRE(&entity == entities[index]);
		REQUIRE(&entity == &set[index]);
		++index;
	}
}


TEST_CASE("Splice inter-container", "[GameLogic:EntitySchemeSet]") {
	Scene scene;
	EntitySchemeSet set1(scene);
	EntitySchemeSet set2(scene);

	set1.SetComponentTypes<bool, int>();
	set2.SetComponentTypes<bool, int>();

	Entity& ent1 = set2.Create(false, 1);
	Entity& ent2 = set2.Create(false, 2);
	Entity& ent3 = set2.Create(false, 3);

	REQUIRE(ent1.GetSet() == &set2);
	REQUIRE(ent2.GetSet() == &set2);
	REQUIRE(ent3.GetSet() == &set2);

	set1.Splice(set2, 2);
	REQUIRE(set1.Size() == 1);
	REQUIRE(set2.Size() == 2);

	set1.Splice(set2, 0);
	REQUIRE(set1.Size() == 2);
	REQUIRE(set2.Size() == 1);

	set1.Splice(set2, 0);
	REQUIRE(set1.Size() == 3);
	REQUIRE(set2.Size() == 0);

	REQUIRE(ent1.GetSet() == &set1);
	REQUIRE(ent2.GetSet() == &set1);
	REQUIRE(ent3.GetSet() == &set1);

	REQUIRE(set1.GetMatrix().entities.size() == 3);
	REQUIRE(set2.GetMatrix().entities.size() == 0);

	REQUIRE(ent1.GetFirstComponent<int>() == 1);
	REQUIRE(ent2.GetFirstComponent<int>() == 2);
	REQUIRE(ent3.GetFirstComponent<int>() == 3);
}


TEST_CASE("Splice intra-container", "[GameLogic:EntitySchemeSet]") {
	Scene scene;
	EntitySchemeSet set1(scene);

	set1.SetComponentTypes<bool, int>();

	Entity& ent1 = set1.Create(false, 1);
	Entity& ent2 = set1.Create(false, 2);

	set1.Splice(set1, 1);
	REQUIRE(set1.Size() == 2);

	set1.Splice(set1, 0);
	REQUIRE(set1.Size() == 2);

	REQUIRE(ent1.GetSet() == &set1);
	REQUIRE(ent2.GetSet() == &set1);

	REQUIRE(set1.GetMatrix().entities.size() == 2);

	REQUIRE(ent1.GetFirstComponent<int>() == 1);
	REQUIRE(ent2.GetFirstComponent<int>() == 2);
}


TEST_CASE("Splice extend", "[GameLogic:EntitySchemeSet]") {
	Scene scene;
	EntitySchemeSet set1(scene);
	EntitySchemeSet set2(scene);

	set1.SetComponentTypes<char, int>();
	set2.SetComponentTypes<int>();

	Entity& ent1 = set2.Create(1);

	set1.SpliceExtend(set2, 0, (char)'a');

	REQUIRE(ent1.GetSet() == &set1);

	REQUIRE(set1.GetMatrix().entities.size() == 1);
	REQUIRE(set2.GetMatrix().entities.size() == 0);

	REQUIRE(ent1.GetFirstComponent<char>() == 'a');
	REQUIRE(ent1.GetFirstComponent<int>() == 1);
}


TEST_CASE("Splice reduce", "[GameLogic:EntitySchemeSet]") {
	Scene scene;
	EntitySchemeSet set1(scene);
	EntitySchemeSet set2(scene);

	set1.SetComponentTypes<int>();
	set2.SetComponentTypes<char, int>();

	Entity& ent1 = set2.Create((char)'a', 1);

	set1.SpliceReduce(set2, 0, 0);

	REQUIRE(ent1.GetSet() == &set1);

	REQUIRE(set1.GetMatrix().entities.size() == 1);
	REQUIRE(set2.GetMatrix().entities.size() == 0);

	REQUIRE(!ent1.HasComponent<char>());
	REQUIRE(ent1.GetFirstComponent<int>() == 1);
}


TEST_CASE("Concatenate sets", "[GameLogic:EntitySchemeSet]") {
	Scene scene1;
	Scene scene2;
	EntitySchemeSet set1(scene1);
	EntitySchemeSet set2(scene2);

	set1.SetComponentTypes<int>();
	set2.SetComponentTypes<int>();

	Entity& ent1 = set1.Create(1);
	Entity& ent2 = set2.Create(2);

	set1 += std::move(set2);

	REQUIRE(set1.Size() == 2);
	REQUIRE(set2.Size() == 0);

	REQUIRE(ent1.GetScene() == &scene1);
	REQUIRE(ent2.GetScene() == &scene1);

	REQUIRE(ent1.GetSet() == &set1);
	REQUIRE(ent2.GetSet() == &set1);

	REQUIRE(ent1.GetFirstComponent<int>() == 1);
	REQUIRE(ent2.GetFirstComponent<int>() == 2);
}