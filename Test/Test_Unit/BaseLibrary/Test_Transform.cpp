#include <BaseLibrary/Transform.hpp>

#include <Catch2/catch.hpp>

using namespace inl;


// Basic absolute setters
TEST_CASE("Set position", "[Transformable]") {
	Transform3D t;
	t.SetPosition({ 1, 2, 3 });
	REQUIRE(ApproxVec(t.GetPosition()) == Vec3{ 1, 2, 3 });
}


TEST_CASE("Set scale", "[Transformable]") {
	Transform3D t;
	t.SetScale({ 1, 2, 3 });
	REQUIRE(ApproxVec(t.GetScale()) == Vec3{ 1, 2, 3 });
}


TEST_CASE("Set rotation", "[Transformable]") {
	Transform3D t;
	Quat rot = RotationAxisAngle(Normalize(Vec3{ 1, 2, 3 }), 0.5f);
	t.SetRotation(rot);
	REQUIRE(ApproxVec(t.GetRotation()) == rot);
}


// Complex absolute setters
TEST_CASE("Set linear", "[Transformable]") {
	Transform3D t;

	Vec3 scale = { 3, 4, 5 };
	Quat rot = RotationAxisAngle(Normalize(Vec3{ 1, 2, 3 }), 0.5f);

	Mat33 linear = Mat33(Scale(scale)) * Mat33(rot);
	t.SetLinearMatrix(linear);

	REQUIRE(ApproxVec(t.GetScale()) == scale);
	REQUIRE(ApproxVec(t.GetRotation()) == rot);
	REQUIRE(ApproxVec(t.GetShearRotation()) == Quat(Identity()));
}


TEST_CASE("Set homogeneous", "[Transformable]") {
	Transform3D t;

	Vec3 scale = { 3, 4, 5 };
	Quat rot = RotationAxisAngle(Normalize(Vec3{ 1, 2, 3 }), 0.5f);
	Vec3 pos = { 9, 8, 7 };

	Mat44 hom = Mat44(Scale(scale)) * Mat44(rot) * Mat44(Translation(pos));
	t.SetMatrix(hom);

	REQUIRE(ApproxVec(t.GetScale()) == scale);
	REQUIRE(ApproxVec(t.GetRotation()) == rot);
	REQUIRE(ApproxVec(t.GetShearRotation()) == Quat(Identity()));
	REQUIRE(ApproxVec(t.GetPosition()) == pos);
}


// Complex getters
TEST_CASE("Get linear", "[Transformable]") {
	Transform3D t;

	Vec3 scale = { 3, 4, 5 };
	Quat rot = RotationAxisAngle(Normalize(Vec3{ 1, 2, 3 }), 0.5f);

	Mat33 linear = Mat33(Scale(scale)) * Mat33(rot);
	t.SetLinearMatrix(linear);

	REQUIRE(ApproxVec(t.GetLinearMatrix()) == linear);
}


TEST_CASE("Get homogeneous", "[Transformable]") {
	Transform3D t;

	Vec3 scale = { 3, 4, 5 };
	Quat rot = RotationAxisAngle(Normalize(Vec3{ 1, 2, 3 }), 0.5f);
	Vec3 pos = { 9, 8, 7 };

	Mat44 hom = Mat44(Scale(scale)) * Mat44(rot) * Mat44(Translation(pos));
	t.SetMatrix(hom);

	REQUIRE(ApproxVec(t.GetMatrix()) == hom);
}


// Relative transforms
TEST_CASE("Relative", "[Transformable]") {
	Mat44 t1 = Translation(1, 2, 3);
	Mat44 t2 = RotationX(Deg2Rad(30.f));
	Mat44 t3 = Scale(1.5f, 0.7f, 1.5f);
	Mat44 t4 = RotationY(Deg2Rad(30.f));

	Mat44 total = t1 * t2 * t3 * t4;

	Transform3D t;
	t.Move({ 1, 2, 3 });
	t.Rotate(RotationAxisAngle(Vec3(1, 0, 0), Deg2Rad(30.f)));
	t.Scale({ 1.5f, 0.7f, 1.5f });
	t.Rotate(RotationAxisAngle(Vec3(0, 1, 0), Deg2Rad(30.f)));

	REQUIRE(ApproxVec(t.GetMatrix()) == total);
}


TEST_CASE("Shear", "[Transformable]") {
	Mat44 t1 = Translation(1, 2, 3);
	Mat44 t2 = Shear(0.4f, 1, 2);
	Mat44 t3 = Shear(0.4f, 0, 1);

	Mat44 total = t1 * t2 * t3;

	Transform3D t;
	t.Move({ 1, 2, 3 });
	t.Shear(0.4f, 1, 2);
	t.Shear(0.4f, 0, 1);

	REQUIRE(ApproxVec(t.GetMatrix()) == total);
}

TEST_CASE("Set rotation 2D", "[Transformable]") {
	using Catch::Detail::Approx;

	Transform2D t;

	t.SetRotation(1.f);

	REQUIRE(t.GetRotation() == Approx(1.f));
}


TEST_CASE("Relative rotation 2D", "[Transformable]") {
	using Catch::Detail::Approx;

	Transform2D t;

	t.Rotate(1.f);

	REQUIRE(t.GetRotation() == Approx(1.f));
}


TEST_CASE("Rotation set by matrix 2D", "[Transformable]") {
	using Catch::Detail::Approx;

	Transform2D t;
	Mat22 rot = Rotation(1.f);

	t.SetLinearMatrix(rot);

	REQUIRE(t.GetRotation() == Approx(1.f));
}


TEST_CASE("Complex transform 2D", "[Transformable]") {
	Mat33 t1 = Translation(1, 2);
	Mat33 t2 = Rotation(Deg2Rad(40.f));
	Mat33 t3 = Scale(4, 0.5f);
	Mat33 t4 = Shear(0.4f, 0, 1);

	Mat33 total = t1 * t2 * t3 * t4;

	Transform2D t;
	t.Move({ 1, 2 });
	t.Rotate(Deg2Rad(40.f));
	t.Scale({ 4, 0.5f });
	t.Shear(0.4f, 0, 1);

	REQUIRE(ApproxVec(t.GetMatrix()) == total);
}