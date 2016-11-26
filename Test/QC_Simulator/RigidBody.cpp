#include "RigidBody.hpp"
#include <iostream>

using namespace mathfu;
using namespace std;


RigidBody::RigidBody() {
	StopMotion();
}

void RigidBody::StopMotion() {
	v = { 0,0,0 };
	dq = { 0,0,0,0 };
	w_ = { 0,0,0 };
	w = { 0,0,0 };
}


void RigidBody::Update(float timestep, mathfu::Vector3f F_, mathfu::Vector3f T_) {
	// advance position
	Vector3f Fdrag;
	float linv = v.Length();
	Fdrag = 0.5f * Cd * 1.225f * linv * -v;
	Vector3f Fgravity = m*G;
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
	Vector4f dq_vec = { dq[0], dq[1], dq[2], dq[3] };
	w_ = q.Inverse() * w;
	mathfu::Vector3f Tdrag_ = -0.1f*w_*w_.Length();
	T_ += Tdrag_;
	auto dw_ = I.Inverse()*T_ - I.Inverse()*Vector3f::CrossProduct(w_, I*w_);
	w_ = w_ + timestep*dw_;
	dq_vec = 0.5f * (G.Transpose() * w_);

	q_vec = q_vec + timestep * dq_vec;
	q_vec.Normalize();
	q = {q_vec[0], q_vec[1], q_vec[2], q_vec[3]};
	w = q*w_;

	//cout << "p = [" << q[0] << "\t" << q[1] << "\t" << q[2] << "\t" << q[3]<< endl;
	//cout << "q = [" << p[0] << "\t" << p[1] << "\t" << p[2] << endl;
}