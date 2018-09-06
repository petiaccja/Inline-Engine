#include <BaseLibrary/Rect.hpp>

#include <Catch2/catch.hpp>

using namespace inl;


TEST_CASE("Defctor", "[Rect]") {
	RectF rc;
	REQUIRE(rc.bottom == 0);
	REQUIRE(rc.top == 0);
	REQUIRE(rc.left == 0);
	REQUIRE(rc.right == 0);
}


TEST_CASE("Field ctor", "[Rect]") {
	RectF rc(1,2,3,4);
	REQUIRE(rc.left == 1);
	REQUIRE(rc.right == 2);
	REQUIRE(rc.bottom == 3);
	REQUIRE(rc.top == 4);
}


TEST_CASE("Point ctor", "[Rect]") {
	RectF rc(Vec2(1,2), Vec2(2,4));
	REQUIRE(rc.left == 1);
	REQUIRE(rc.right == 2);
	REQUIRE(rc.bottom == 2);
	REQUIRE(rc.top == 4);
}


TEST_CASE("Size ctor", "[Rect]") {
	RectF rc = RectF::FromSize(Vec2(1,2), Vec2(2,4));
	REQUIRE(rc.left == 1);
	REQUIRE(rc.right == 3);
	REQUIRE(rc.bottom == 2);
	REQUIRE(rc.top == 6);
}


TEST_CASE("GetSize", "[Rect]") {
	RectF rc = RectF::FromSize(Vec2(1, 2), Vec2(2, 4));
	Vec2 expected(2, 4);
	REQUIRE(rc.GetSize() == expected.Approx());
}


TEST_CASE("GetWidth/GetHeight", "[Rect]") {
	RectF rc = RectF::FromSize(Vec2(1, 2), Vec2(2, 4));
	REQUIRE(rc.GetWidth() == Approx(2));
	REQUIRE(rc.GetHeight() == Approx(4));
}


TEST_CASE("GetArea", "[Rect]") {
	RectF rc = RectF::FromSize(Vec2(1, 2), Vec2(2, 4));
	REQUIRE(rc.GetArea() == Approx(8));
}


TEST_CASE("SetSize", "[Rect]") {
	// Calls SetWidth and SetHeight.
	RectF rc(Vec2(-1, -1), Vec2(1, 1));
	rc.SetSize({ 3,4 }, { 0.25f, 0.5f });

	REQUIRE(rc.GetWidth() == Approx(3));
	REQUIRE(rc.GetHeight() == Approx(4));
	Vec2 expected(0.25f, 0);
	REQUIRE(rc.GetCenter() == expected.Approx());
}


TEST_CASE("Union", "[Rect]") {
	RectF rc1(Vec2(-1, -1), Vec2(1, 1));
	RectF rc2(Vec2(0, 0), Vec2(3, 2));

	auto rcu = RectF::Union(rc1, rc2);

	Vec2 exp1(-1, -1);
	Vec2 exp2(3, 2);
	REQUIRE(rcu.GetBottomLeft() == exp1.Approx());
	REQUIRE(rcu.GetTopRight() == exp2.Approx());
}


TEST_CASE("Intersection", "[Rect]") {
	RectF rc1(Vec2(-1, -1), Vec2(1, 1));
	RectF rc2(Vec2(0, 0), Vec2(3, 2));

	auto rcu = RectF::Intersection(rc1, rc2);

	Vec2 exp1(0, 0);
	Vec2 exp2(1, 1);
	REQUIRE(rcu.GetBottomLeft() == exp1.Approx());
	REQUIRE(rcu.GetTopRight() == exp2.Approx());
}


TEST_CASE("Is point inside", "[Rect]") {
	RectF rc(Vec2(-2, -1), Vec2(1, 3));
	REQUIRE(!rc.IsPointInside({0,4}));
	REQUIRE(!rc.IsPointInside({1.5f,3}));
	REQUIRE(rc.IsPointInside({0,0}));
}


TEST_CASE("Is rect inside", "[Rect]") {
	RectF rc(Vec2(-2, -1), Vec2(1, 3));
	REQUIRE(!rc.IsRectInside(RectF({ 0,4 }, {3,8})));
	REQUIRE(!rc.IsRectInside(RectF({ 0,0 }, { 3, 3 })));
	REQUIRE(rc.IsRectInside(RectF({ 0,0 }, { 0.5f, 0.5f })));
}


TEST_CASE("Is intersecting", "[Rect]") {
	RectF rc(Vec2(-2, -1), Vec2(1, 3));
	REQUIRE(!rc.IsIntersecting(RectF({ 0,4 }, { 3,8 })));
	REQUIRE(rc.IsIntersecting(RectF({ 0,0 }, { 3, 3 })));
}