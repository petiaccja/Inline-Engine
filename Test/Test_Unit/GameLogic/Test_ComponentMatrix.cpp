#include "Components.hpp"

#include <GameLogic/ComponentMatrix.hpp>

#include <Catch2/catch.hpp>

using namespace inl::game;


TEST_CASE("Ctor", "[GameLogic:ComponentMatrix]") {
	ComponentMatrix matrix;
	REQUIRE(matrix.entities.size() == 0);
	REQUIRE(matrix.types.size() == 0);
}


TEST_CASE("Add types", "[GameLogic:ComponentMatrix]") {
	ComponentMatrix matrix;
	matrix.types.insert(matrix.types.begin(), ComponentVector<FooComponent>{});

	REQUIRE(matrix.entities.size() == 0);
	REQUIRE(matrix.types.size() == 1);
	REQUIRE(matrix.types.begin()->get_type() == typeid(FooComponent));
}


TEST_CASE("Add multiple in order", "[GameLogic:ComponentMatrix]") {
	ComponentMatrix matrix;
	matrix.types.insert(matrix.types.end(), ComponentVector<FooComponent>{});
	matrix.types.insert(matrix.types.end(), ComponentVector<BarComponent>{});
	matrix.types.insert(matrix.types.end(), ComponentVector<BazComponent>{});

	REQUIRE(matrix.entities.size() == 0);
	REQUIRE(matrix.types.size() == 3);
	REQUIRE((matrix.types[0]).get_type() == typeid(FooComponent));
	REQUIRE((matrix.types[1]).get_type() == typeid(BarComponent));
	REQUIRE((matrix.types[2]).get_type() == typeid(BazComponent));
}


TEST_CASE("Remove types matrix", "[GameLogic:ComponentMatrix]") {
	ComponentMatrix matrix;
	matrix.types.insert(matrix.types.end(), ComponentVector<FooComponent>{});
	matrix.types.insert(matrix.types.end(), ComponentVector<BarComponent>{});
	matrix.types.insert(matrix.types.end(), ComponentVector<BazComponent>{});

	matrix.types.erase(matrix.types.begin() + 1);

	REQUIRE(matrix.entities.size() == 0);
	REQUIRE(matrix.types.size() == 2);
	REQUIRE(matrix.types[0].get_type() == typeid(FooComponent));
	REQUIRE(matrix.types[1].get_type() == typeid(BazComponent));
}


TEST_CASE("Insert entity empty types", "[GameLogic:ComponentMatrix]") {
	ComponentMatrix matrix;
	REQUIRE_THROWS(matrix.entities.insert(matrix.entities.begin(), {}));
}


TEST_CASE("Insert entity empty ref", "[GameLogic:ComponentMatrix]") {
	ComponentMatrix matrix;
	matrix.types.insert(matrix.types.end(), ComponentVector<FooComponent>{});
	matrix.types.insert(matrix.types.end(), ComponentVector<BarComponent>{});

	matrix.entities.insert(matrix.entities.begin(), {});

	REQUIRE(matrix.entities.size() == 1);
	REQUIRE(matrix.entities.begin()->size() == 2);
	REQUIRE(matrix.entities.begin()->get_type(0) == typeid(FooComponent));
	REQUIRE(matrix.entities.begin()->get_type(1) == typeid(BarComponent));
}


TEST_CASE("Insert entity copy ref", "[GameLogic:ComponentMatrix]") {
	ComponentMatrix matrix;
	matrix.types.insert(matrix.types.end(), ComponentVector<FooComponent>{});
	matrix.types.insert(matrix.types.end(), ComponentVector<BarComponent>{});

	matrix.entities.insert(matrix.entities.end(), {});
	matrix.entities[0].get<FooComponent>(0).value = 13.f;
	matrix.entities[0].get<BarComponent>(1).value = 12.f;

	matrix.entities.insert(matrix.entities.end(), *matrix.entities.cbegin());

	REQUIRE(matrix.entities[1].get<FooComponent>(0).value == 13.f);
	REQUIRE(matrix.entities[1].get<BarComponent>(1).value == 12.f);
}


TEST_CASE("Emplace entity", "[GameLogic:ComponentMatrix]") {
	ComponentMatrix matrix;
	matrix.types.insert(matrix.types.end(), ComponentVector<FooComponent>{});
	matrix.types.insert(matrix.types.end(), ComponentVector<BarComponent>{});

	matrix.entities.emplace(matrix.entities.end(), FooComponent{ 13.f }, BarComponent{ 12.f });
	REQUIRE(matrix.entities[0].get<FooComponent>(0).value == 13.f);
	REQUIRE(matrix.entities[0].get<BarComponent>(1).value == 12.f);
}


TEST_CASE("Emplace entity partial", "[GameLogic:ComponentMatrix]") {
	using namespace inl::game;
	ComponentMatrix matrix;
	matrix.types.insert(matrix.types.end(), ComponentVector<FooComponent>{});
	matrix.types.insert(matrix.types.end(), ComponentVector<BarComponent>{});
	matrix.types.insert(matrix.types.end(), ComponentVector<BazComponent>{});

	matrix.entities.emplace(matrix.entities.end(), FooComponent{ 13.f }, BarComponent{ 12.f });
	REQUIRE(matrix.entities[0].get<FooComponent>(0).value == 13.f);
	REQUIRE(matrix.entities[0].get<BarComponent>(1).value == 12.f);

	matrix.entities.emplace(matrix.entities.end(), FooComponent{ 16.f }, BazComponent{ 15.f });
	REQUIRE(matrix.entities[1].get<FooComponent>(0).value == 16.f);
	REQUIRE(matrix.entities[1].get<BazComponent>(2).value == 15.f);

	for (auto it = matrix.types.begin(); it != matrix.types.end(); ++it) {
		REQUIRE(it->size() == 2);
	}
}


TEST_CASE("Insert entity copy ref partial", "[GameLogic:ComponentMatrix]") {
	ComponentMatrix srcMatrix;
	srcMatrix.types.insert(srcMatrix.types.end(), ComponentVector<FooComponent>{});
	srcMatrix.types.insert(srcMatrix.types.end(), ComponentVector<BarComponent>{});

	srcMatrix.entities.emplace(srcMatrix.entities.end(), FooComponent{ 13.f }, BarComponent{ 12.f });

	ComponentMatrix tarMatrix;
	tarMatrix.types.insert(tarMatrix.types.end(), ComponentVector<FooComponent>{});
	tarMatrix.types.insert(tarMatrix.types.end(), ComponentVector<BarComponent>{});
	tarMatrix.types.insert(tarMatrix.types.end(), ComponentVector<BazComponent>{});

	tarMatrix.entities.insert(tarMatrix.entities.end(), srcMatrix.entities[0]);

	for (auto it = tarMatrix.types.begin(); it != tarMatrix.types.end(); ++it) {
		REQUIRE(it->size() == 1);
	}
}


TEST_CASE("Add type non-empty", "[GameLogic:ComponentMatrix]") {
	ComponentMatrix matrix;
	matrix.types.insert(matrix.types.end(), ComponentVector<FooComponent>{});
	matrix.types.insert(matrix.types.end(), ComponentVector<BarComponent>{});

	matrix.entities.emplace(matrix.entities.end(), FooComponent{ 13.f }, BarComponent{ 12.f });

	matrix.types.insert(matrix.types.end(), ComponentVector<BazComponent>{});

	for (auto it = matrix.types.begin(); it != matrix.types.end(); ++it) {
		REQUIRE(it->size() == 1);
	}
}


TEST_CASE("Types assign empty", "[GameLogic:ComponentMatrix]") {
	ComponentMatrix matrix1;

	matrix1.types.insert(matrix1.types.end(), ComponentVector<FooComponent>{});
	matrix1.types.insert(matrix1.types.end(), ComponentVector<BarComponent>{});

	ComponentMatrix matrix2;
	matrix2.types = matrix1.types;

	REQUIRE(matrix1.types.size() == matrix2.types.size());
	REQUIRE(matrix1.types.type_order().size() == matrix1.types.size());
	REQUIRE(matrix2.types.type_order().size() == matrix2.types.size());
	const size_t end = matrix1.types.size();
	for (size_t i = 0; i < end; ++i) {
		REQUIRE(matrix1.types.type_order()[i].first == matrix2.types.type_order()[i].first);
	}
}


TEST_CASE("Types assign subset", "[GameLogic:ComponentMatrix]") {
	ComponentMatrix matrix1;

	matrix1.types.insert(matrix1.types.end(), ComponentVector<FooComponent>{});
	matrix1.types.insert(matrix1.types.end(), ComponentVector<BarComponent>{});

	ComponentMatrix matrix2;
	matrix2.types.insert(matrix1.types.end(), ComponentVector<BarComponent>{});
	matrix2.types = matrix1.types;

	REQUIRE(matrix1.types.size() == matrix2.types.size());
	REQUIRE(matrix1.types.type_order().size() == matrix1.types.size());
	REQUIRE(matrix2.types.type_order().size() == matrix2.types.size());
	const size_t end = matrix1.types.size();
	for (size_t i = 0; i < end; ++i) {
		REQUIRE(matrix1.types.type_order()[i].first == matrix2.types.type_order()[i].first);
	}
}

TEST_CASE("Types assign superset", "[GameLogic:ComponentMatrix]") {
	ComponentMatrix matrix1;

	matrix1.types.insert(matrix1.types.end(), ComponentVector<FooComponent>{});
	matrix1.types.insert(matrix1.types.end(), ComponentVector<BarComponent>{});

	ComponentMatrix matrix2;
	matrix2.types.insert(matrix1.types.end(), ComponentVector<FooComponent>{});
	matrix2.types.insert(matrix1.types.end(), ComponentVector<BarComponent>{});
	matrix2.types.insert(matrix1.types.end(), ComponentVector<BarComponent>{});
	matrix2.types.insert(matrix1.types.end(), ComponentVector<BazComponent>{});

	matrix2.types = matrix1.types;

	REQUIRE(matrix1.types.size() == matrix2.types.size());
	REQUIRE(matrix1.types.type_order().size() == matrix1.types.size());
	REQUIRE(matrix2.types.type_order().size() == matrix2.types.size());
	const size_t end = matrix1.types.size();
	for (size_t i = 0; i < end; ++i) {
		REQUIRE(matrix1.types.type_order()[i].first == matrix2.types.type_order()[i].first);
	}
}
