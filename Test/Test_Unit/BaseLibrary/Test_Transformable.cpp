#include <BaseLibrary/Transformable.hpp>

#include <Catch2/catch.hpp>

using namespace inl;


// Basic absolute setters
TEST_CASE("Set position", "[Transformable]") {
	Transformable3D t;
	t.SetPosition({ 1, 2, 3 });
	REQUIRE(t.GetPosition().Approx() == Vec3{ 1, 2, 3 });
}


TEST_CASE("Set scale", "[Transformable]") {
	Transformable3D t;
	t.SetScale({ 1, 2, 3 });
	REQUIRE(t.GetScale().Approx() == Vec3{ 1, 2, 3 });
}


TEST_CASE("Set rotation", "[Transformable]") {
	Transformable3D t;
	Quat rot = Quat::AxisAngle(Vec3{ 1, 2, 3 }.Normalized(), 0.5f);
	t.SetRotation(rot);
	REQUIRE(t.GetRotation().Approx() == rot);
}


// Complex absolute setters
TEST_CASE("Set linear", "[Transformable]") {
	Transformable3D t;

	Vec3 scale = { 3, 4, 5 };
	Quat rot = Quat::AxisAngle(Vec3{ 1, 2, 3 }.Normalized(), 0.5f);

	Mat33 linear = Mat33::Scale(scale) * Mat33(rot);
	t.SetLinearTransform(linear);

	REQUIRE(t.GetScale().Approx() == scale);
	REQUIRE(t.GetRotation().Approx() == rot);
	REQUIRE(t.GetShearRotation().Approx() == Quat::Identity());
}


TEST_CASE("Set homogeneous", "[Transformable]") {
	Transformable3D t;

	Vec3 scale = { 3, 4, 5 };
	Quat rot = Quat::AxisAngle(Vec3{ 1, 2, 3 }.Normalized(), 0.5f);
	Vec3 pos = { 9, 8, 7 };

	Mat44 hom = Mat44::Scale(scale) * Mat44(rot) * Mat44::Translation(pos);
	t.SetTransform(hom);

	REQUIRE(t.GetScale().Approx() == scale);
	REQUIRE(t.GetRotation().Approx() == rot);
	REQUIRE(t.GetShearRotation().Approx() == (Quat::Identity()));
	REQUIRE(t.GetPosition().Approx() == pos);
}


// Complex getters
TEST_CASE("Get linear", "[Transformable]") {
	Transformable3D t;

	Vec3 scale = { 3, 4, 5 };
	Quat rot = Quat::AxisAngle(Vec3{ 1, 2, 3 }.Normalized(), 0.5f);

	Mat33 linear = Mat33::Scale(scale) * Mat33(rot);
	t.SetLinearTransform(linear);

	REQUIRE(t.GetLinearTransform().Approx() == linear);
}


TEST_CASE("Get homogeneous", "[Transformable]") {
	Transformable3D t;

	Vec3 scale = { 3, 4, 5 };
	Quat rot = Quat::AxisAngle(Vec3{ 1, 2, 3 }.Normalized(), 0.5f);
	Vec3 pos = { 9, 8, 7 };

	Mat44 hom = Mat44::Scale(scale) * Mat44(rot) * Mat44::Translation(pos);
	t.SetTransform(hom);

	REQUIRE(t.GetTransform().Approx() == hom);
}


// Relative transforms
TEST_CASE("Relative", "[Transformable]") {
	Mat44 t1 = Mat44::Translation(1, 2, 3);
	Mat44 t2 = Mat44::RotationX(Deg2Rad(30.f));
	Mat44 t3 = Mat44::Scale(1.5f, 0.7f, 1.5f);
	Mat44 t4 = Mat44::RotationY(Deg2Rad(30.f));

	Mat44 total = t1 * t2 * t3 * t4;

	Transformable3D t;
	t.Move({ 1, 2, 3 });
	t.Rotate(Quat::AxisAngle(Vec3(1, 0, 0), Deg2Rad(30.f)));
	t.Scale({ 1.5f, 0.7f, 1.5f });
	t.Rotate(Quat::AxisAngle(Vec3(0, 1, 0), Deg2Rad(30.f)));

	REQUIRE(t.GetTransform().Approx() == total);
}


TEST_CASE("Shear", "[Transformable]") {
	Mat44 t1 = Mat44::Translation(1, 2, 3);
	Mat44 t2 = Mat44::Shear(0.4f, 1, 2);
	Mat44 t3 = Mat44::Shear(0.4f, 0, 1);

	Mat44 total = t1 * t2 * t3;

	Transformable3D t;
	t.Move({ 1, 2, 3 });
	t.Shear(0.4f, 1, 2);
	t.Shear(0.4f, 0, 1);

	REQUIRE(t.GetTransform().Approx() == total);
}

TEST_CASE("Set rotation 2D", "[Transformable]") {
	using Catch::Detail::Approx;

	Transformable2D t;

	t.SetRotation(1.f);

	REQUIRE(t.GetRotation() == Approx(1.f));
}


TEST_CASE("Relative rotation 2D", "[Transformable]") {
	using Catch::Detail::Approx;

	Transformable2D t;

	t.Rotate(1.f);

	REQUIRE(t.GetRotation() == Approx(1.f));
}


TEST_CASE("Rotation set by matrix 2D", "[Transformable]") {
	using Catch::Detail::Approx;

	Transformable2D t;
	Mat22 rot = Mat22::Rotation(1.f);

	t.SetLinearTransform(rot);

	REQUIRE(t.GetRotation() == Approx(1.f));
}


TEST_CASE("Complex transform 2D", "[Transformable]") {
	Mat33 t1 = Mat33::Translation(1, 2);
	Mat33 t2 = Mat33::Rotation(Deg2Rad(40.f));
	Mat33 t3 = Mat33::Scale(4, 0.5f);
	Mat33 t4 = Mat33::Shear(0.4f, 0, 1);

	Mat33 total = t1 * t2 * t3 * t4;

	Transformable2D t;
	t.Move({ 1, 2 });
	t.Rotate(Deg2Rad(40.f));
	t.Scale({ 4, 0.5f });
	t.Shear(0.4f, 0, 1);

	REQUIRE(t.GetTransform().Approx() == total);
}



// Motion estimation
TEST_CASE("Motion estimation shear", "[Transformable]") {
	Mat33 t1 = Mat33::Shear(0.5f, 0, 1);
	Mat33 t2 = Mat33::Shear(0.6f, 0, 1);
	float deltaTime = 0.1f;

	Transformable3D t;

	t.SetLinearTransform(t1);
	t.SetPosition(Vec3(1.f, 2.f, 3.f));

	t.UpdateTransformMotion(deltaTime);

	t.SetLinearTransform(t2);
	t.SetPosition(Vec3(2.f, 2.5f, 4.f));

	Mat44 motion = t.GetTransformMotion();
	Mat44 expected = {
		0,
		0,
		0,
		0,
		1,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		10,
		5,
		10,
		0,
	};
	REQUIRE(motion.Approx() == expected);

	Vec3 velocity1 = (Vec3)(Vec4(0, 0, 0, 1) * motion);
	Vec3 velocity2 = (Vec3)(Vec4(0, 5, 0, 1) * motion);
	REQUIRE(velocity1.Approx() == Vec3(10, 5, 10));
	REQUIRE(velocity2.Approx() == Vec3(15, 5, 10));
}


TEST_CASE("Motion estimation rotate", "[Transformable]") {
	double deltaTime = 0.0001;
	double angular = 1;
	double angle = angular * deltaTime;

	Vec3d pos = Vec3d(10, 0, 0);
	Mat44d T = Mat44d::Translation(pos);
	Vec3d axis = Vec3d(10, 10, 10).Normalized();
	Mat44d R1 = Mat44d::RotationAxisAngle(axis, Deg2Rad(0));
	Mat44d R2 = Mat44d::RotationAxisAngle(axis, Deg2Rad(angle));

	Vec3d velocity = Cross(axis, pos) * Deg2Rad(angle);

	Transformable3Dd t;
	t.SetTransform(T * R1);
	t.UpdateTransformMotion(1);
	t.SetTransform(T * R2);

	Mat44d motion = t.GetTransformMotion();

	Vec3d velmat(Vec4d(0, 0, 0, 1) * motion);

	REQUIRE(velmat.Approx() == velocity);
}


TEST_CASE("Motion concat", "[Transformable]") {
	Mat44 A1 = Mat44::RotationAxisAngle(Vec3(1, 0, 0).Normalized(), Deg2Rad(90.f)) * Mat44::Translation(1, 2, 3);
	Mat44 B1 = Mat44::Translation(4, 3, 2);

	Mat44 A2 = Mat44::RotationAxisAngle(Vec3(0.98, 0.02, 0).Normalized(), Deg2Rad(85.f)) * Mat44::Translation(1, 2, 3);
	Mat44 B2 = Mat44::Translation(4.1, 3.1, 2.1);

	Mat44 M1 = A1 * B1;
	Mat44 M2 = A2 * B2;
	float deltaTime = 0.1f;


	Transformable3D ta, tb, tm;

	ta.SetTransform(A1);
	ta.UpdateTransformMotion(deltaTime);
	ta.SetTransform(A2);

	tb.SetTransform(B1);
	tb.UpdateTransformMotion(deltaTime);
	tb.SetTransform(B2);

	tm.SetTransform(M1);
	tm.UpdateTransformMotion(deltaTime);
	tm.SetTransform(M2);

	Mat44 motiona = ta.GetTransformMotion();
	Mat44 motionb = tb.GetTransformMotion();
	Mat44 motionm = tm.GetTransformMotion();

	Mat44 motionProductRule = motiona * B2 + A2 * motionb;

	REQUIRE(motionProductRule.Approx() == motionm);
}