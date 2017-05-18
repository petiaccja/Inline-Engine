#pragma once


#include <mathfu/mathfu_exc.hpp>


class RigidBody {
public:
	RigidBody();

	void Update(float timestep, mathfu::Vector3f F_, mathfu::Vector3f T_);

	void SetPosition(mathfu::Vector3f p) { this->p = p; }
	void SetRotation(mathfu::Quaternionf q) { this->q = q; }

	mathfu::Vector3f GetPosition() const { return p; }
	mathfu::Vector3f GetVelocity() const { return v; }
	mathfu::Quaternionf GetRotation() const { return q; }
	mathfu::Vector3f GetAngularVelocity() const { return w; }

	void SetMass(float mass) { m = mass; }
	void SetInertia(mathfu::Matrix3x3f inertia) { I = inertia; }

	void SetGravity(mathfu::Vector3f gravity) { G = gravity; }

	void StopMotion();
private:
	mathfu::Vector3f p;
	mathfu::Vector3f v;

	mathfu::Quaternionf q;
	mathfu::Quaternionf dq;
	mathfu::Vector3f w_;
	mathfu::Vector3f w;

	float m;
	mathfu::Matrix3x3f I;
	float Cd = 0.6;
	mathfu::Vector3f G;
};
