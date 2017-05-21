#include "RigidBody.hpp"
#include <iostream>

using namespace mathfu;
using namespace std;
using namespace inl;


RigidBody::RigidBody() {
	StopMotion();
}

void RigidBody::StopMotion() {
	v = { 0,0,0 };
	dq = { 0,0,0,0 };
	w_ = { 0,0,0 };
	w = { 0,0,0 };
}


void RigidBody::Update(float timestep, Vec3 F_, Vec3 T_) {
	// advance position
	Vec3 Fdrag;
	float linv = v.Length();
	Fdrag = 0.5f * Cd * 1.225f * linv * -v * 0.1f;
	Vec3 Fgravity = m*G;
	auto qFmfu = q*mathfu::Vector3f(F_.x, F_.y, F_.z);
	inl::Vec3 F = inl::Vec3(qFmfu.x(), qFmfu.y(), qFmfu.z()) + Fdrag + Fgravity;

	auto a = F / m;
	v = v + timestep*a;
	p = p + timestep*v;

	// advance rotation
	Mat34 G = {
		-q[1], q[0], q[3], -q[2],
		-q[2], -q[3], q[0], q[1],
		-q[3], q[2], -q[1], q[0]
	};
	Vec4 q_vec = { q[0], q[1], q[2], q[3] };
	Vec4 dq_vec = { dq[0], dq[1], dq[2], dq[3] };
	auto w_mfu = q.Inverse() * mathfu::Vector3f(w.x, w.y, w.z);
	w_ = { w_mfu.x(), w_mfu.y(), w_mfu.z() };
	Vec3 Tdrag_ = -0.01f*w_*w_.Length();
	T_ += Tdrag_;
	auto dw_ = I.Inverse()*T_ - I.Inverse()*Cross(w_, I*w_);
	w_ = w_ + timestep*dw_;
	dq_vec = 0.5f * (G.Transposed() * w_);

	q_vec = q_vec + timestep * dq_vec;
	q_vec.Normalize();
	q = {q_vec[0], q_vec[1], q_vec[2], q_vec[3]};
	auto wmfu = q*mathfu::Vector3f(w_.x, w_.y, w_.z);
	w = { wmfu.x(), wmfu.y(), wmfu.z() };

	//cout << "p = [" << q[0] << "\t" << q[1] << "\t" << q[2] << "\t" << q[3]<< endl;
	//cout << "q = [" << p[0] << "\t" << p[1] << "\t" << p[2] << endl;
}