#pragma once


#include <InlineMath.hpp>


class RigidBody {
public:
	RigidBody();

	void Update(float timestep, inl::Vec3 F_, inl::Vec3 T_);

	void SetPosition(inl::Vec3 p) { this->p = p; }
	void SetRotation(inl::Quat q) { this->q = q; }

	inl::Vec3 GetPosition() const { return p; }
	inl::Vec3 GetVelocity() const { return v; }
	inl::Quat GetRotation() const { return q; }
	inl::Vec3 GetAngularVelocity() const { return w; }

	void SetMass(float mass) { m = mass; }
	void SetInertia(inl::Mat33 inertia) { I = inertia; }

	void SetGravity(inl::Vec3 gravity) { G = gravity; }

	void StopMotion();
private:
	inl::Vec3 p;
	inl::Vec3 v;

	inl::Quat q;
	inl::Quat dq;
	inl::Vec3 w_;
	inl::Vec3 w;

	float m;
	inl::Mat33 I;
	float Cd = 0.6f;
	inl::Vec3 G;
};
