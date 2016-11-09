#include "RigidBody.hpp"

using namespace mathfu;

void RigidBody::Reset() {
	v = { 0,0,0 };
	dq = { 0,0,0,0 };
	w_ = { 0,0,0 };
}


void RigidBody::Update(float timestep, mathfu::Vector3f F_, mathfu::Vector3f T_) {
	// copter parameters
	float m = 2;
	float arm_len = 0.25;
	float Ixx = arm_len*arm_len / 2 * 0.2 * 4;
	float Iyy = Ixx;
	float Izz = arm_len*arm_len * 0.2 * 4;
	Matrix3x3f I = {
		Ixx, 0, 0,
		0, Iyy, 0,
		0, 0, Izz };

	// advance position
	Vector3f Fdrag = -v * v.Normalized();
	Vector3f Fgravity = { 0, 0, -9.81f };
	Vector3f F = q*F_ + Fdrag + Fgravity;

	auto a = F / m;
	v = v + timestep*a;
	p = p + timestep*v;

	// advance rotation
	Matrix3x4f G = Matrix4x3f{
		-q[1], q[0], q[3], -q[2],
		-q[2], -q[3], q[0], q[1],
		-q[3], q[2], -q[1], q[0]
	}.Transpose();
	Vector4f q_vec = { q[0], q[1], q[2], q[3] };
	w_ = 2.f * (G*q_vec);
	auto dw_ = I.Inverse()*T_ - I.Inverse()*Vector3f::CrossProduct(w_, I*w_);
	w_ = w_ + timestep*dw_;
	Vector4f dq_vec = 0.5f * (G.Transpose() * w_);

	q_vec = q_vec + timestep * dq_vec;
	q_vec.Normalize();
	q = {q_vec[0], q_vec[1], q_vec[2], q_vec[3]};
}