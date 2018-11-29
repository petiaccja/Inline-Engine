#include <GuiEngine/LinearLayout.hpp>
#include <GuiEngine/Button.hpp>

#include <Catch2/catch.hpp>


using namespace inl::gui;


TEST_CASE("LinearLayout ordering horizontal", "[GUI]") {
	LinearLayout layout;
	layout.SetSize({ 200, 200 });
	layout.SetPosition({ 0,0 });

	Button b1, b2, b3;
	layout.PushBack(b1, LinearLayout::CellSize().SetWeight(1));
	layout.PushBack(b2, LinearLayout::CellSize().SetWeight(1));
	layout.PushBack(b3, LinearLayout::CellSize().SetWeight(1));

	layout.UpdateLayout();

	REQUIRE(b1.GetPosition().x < b2.GetPosition().x);
	REQUIRE(b2.GetPosition().x < b3.GetPosition().x);

	REQUIRE(b1.GetPosition().y < 5.f);
	REQUIRE(b1.GetPosition().y > -5.f);
	REQUIRE(b1.GetPosition().y == b2.GetPosition().y);
	REQUIRE(b2.GetPosition().y == b3.GetPosition().y);
}


TEST_CASE("LinearLayout ordering vertical", "[GUI]") {
	LinearLayout layout;
	layout.SetSize({ 200, 200 });
	layout.SetPosition({ 0,0 });
	layout.SetDirection(LinearLayout::VERTICAL);

	Button b1, b2, b3;
	layout.PushBack(b1, LinearLayout::CellSize().SetWeight(1));
	layout.PushBack(b2, LinearLayout::CellSize().SetWeight(1));
	layout.PushBack(b3, LinearLayout::CellSize().SetWeight(1));

	layout.UpdateLayout();

	REQUIRE(b1.GetPosition().y < b2.GetPosition().y);
	REQUIRE(b2.GetPosition().y < b3.GetPosition().y);

	REQUIRE(b1.GetPosition().x < 5.f);
	REQUIRE(b1.GetPosition().x > -5.f);
	REQUIRE(b1.GetPosition().x == b2.GetPosition().x);
	REQUIRE(b2.GetPosition().x == b3.GetPosition().x);
}


TEST_CASE("LinearLayout ordering inverted", "[GUI]") {
	LinearLayout layout;
	layout.SetSize({ 200, 200 });
	layout.SetPosition({ 0,0 });
	layout.SetInverted(true);

	Button b1, b2, b3;
	layout.PushBack(b1, LinearLayout::CellSize().SetWeight(1));
	layout.PushBack(b2, LinearLayout::CellSize().SetWeight(1));
	layout.PushBack(b3, LinearLayout::CellSize().SetWeight(1));

	layout.UpdateLayout();

	REQUIRE(b1.GetPosition().x > b2.GetPosition().x);
	REQUIRE(b2.GetPosition().x > b3.GetPosition().x);

	REQUIRE(b1.GetPosition().y < 5.f);
	REQUIRE(b1.GetPosition().y > -5.f);
	REQUIRE(b1.GetPosition().y == b2.GetPosition().y);
	REQUIRE(b2.GetPosition().y == b3.GetPosition().y);
}


TEST_CASE("LinearLayout margins", "[GUI]") {
	LinearLayout layout;
	layout.SetSize({ 300, 100 });
	layout.SetPosition({ 150, 50 });

	Button b1, b2, b3;
	layout.PushBack(b1, LinearLayout::CellSize().SetWidth(100.f));
	layout.PushBack(b2, LinearLayout::CellSize().SetWidth(100.f));
	layout.PushBack(b3, LinearLayout::CellSize().SetWeight(1));
	layout[0].SetMargin({ 20, 10, 30, 20 });
	layout[1].SetMargin({ 3, 3, 3, 3 });
	layout[2].SetMargin({ 3, 3, 3, 3 });

	layout.UpdateLayout();

	REQUIRE(b1.GetSize().Approx() == Vec2{70.f, 50.f});
	REQUIRE(b1.GetPosition().Approx() == Vec2{55.f, 55.f});

	REQUIRE(b2.GetSize().Approx() == Vec2{ 94.f, 94.f });
	REQUIRE(b2.GetPosition().Approx() == Vec2{ 150.f, 50.f });

	REQUIRE(b3.GetSize().Approx() == Vec2{ 94.f, 94.f });
	REQUIRE(b3.GetPosition().Approx() == Vec2{ 250.f, 50.f });
}